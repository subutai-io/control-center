#include <stdio.h>
#include <sstream>

#include <QHostAddress>
#include <QApplication>
#include <SystemCallWrapper.h>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>

#include "SettingsManager.h"
#include "NotificationObserver.h"
#include "HubController.h"
#include "libssh2/include/LibsshController.h"
#include "OsBranchConsts.h"
#include "VBoxManager.h"
#include "ApplicationLog.h"

static QString error_strings[] = {
  "Success", "Shell error",
  "Pipe error", "Set handle info error",
  "Create process error", "Cant join to swarm",
  "SSH launch error", "System call timeout"
};

system_call_res_t
CSystemCallWrapper::ssystem_th(const QString &cmd,
                               const QStringList &args,
                               bool read_output,
                               unsigned long timeout_msec) {

  QFuture<system_call_res_t> f1 = QtConcurrent::run(
                                    ssystem, cmd, args,
                                    read_output, timeout_msec);
  f1.waitForFinished();
  return f1.result();
}
////////////////////////////////////////////////////////////////////////////

system_call_res_t
CSystemCallWrapper::ssystem(const QString &cmd,
                            const QStringList &args,
                            bool read_out,
                            unsigned long timeout_msec) {
  QProcess proc;
  system_call_res_t res = {SCWE_SUCCESS, QStringList(), 0};

  proc.start(cmd, args);
  if (!proc.waitForStarted(timeout_msec)) {
    CApplicationLog::Instance()->LogError("Failed to wait for started process %s", cmd.toStdString().c_str());
    CApplicationLog::Instance()->LogError("%s", proc.errorString().toStdString().c_str());
    res.res = SCWE_CREATE_PROCESS;
    return res;
  }

  if (!proc.waitForFinished(timeout_msec)) {
    proc.terminate();
    res.res = SCWE_TIMEOUT;
    return res;
  }

  if (read_out) {
    QString output = QString(proc.readAll());
    res.out = output.split("\n", QString::SkipEmptyParts);
  }

  res.exit_code = proc.exitCode();
  res.res = proc.exitStatus() == QProcess::NormalExit ?
              SCWE_SUCCESS : SCWE_PROCESS_CRASHED;
  return res;
}
////////////////////////////////////////////////////////////////////////////

bool
CSystemCallWrapper::is_in_swarm(const QString& hash) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show";
  system_call_res_t res = ssystem_th(cmd, args, true);

  if (res.res != SCWE_SUCCESS && res.exit_code != 1) {
    CNotificationObserver::Error(error_strings[res.res]);
    CApplicationLog::Instance()->LogError(error_strings[res.res].toStdString().c_str());
    return false;
  }

  //todo use indexOf
  bool is_in = false;
  for (auto i = res.out.begin(); i != res.out.end() && !is_in; ++i) {
    is_in |= (i->indexOf(hash) != -1);
  }
  return is_in;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::join_to_p2p_swarm(const QString& hash,
                                      const QString& key,
                                      const QString& ip)
{
  if (is_in_swarm(hash))
    return SCWE_SUCCESS;

  CApplicationLog::Instance()->LogTrace("join to p2p swarm called. hash : %s",
                                        hash.toStdString().c_str());
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "start" <<
          "-ip" << ip <<
          "-key" << key <<
          "-hash" << hash <<
          "-dht" << p2p_dht_arg();

  system_call_res_t res;

  int attempts_count = 5;
  do {
    res = ssystem_th(cmd, args, true);
    if (res.res != SCWE_SUCCESS)
      continue;

    if (res.out.size() == 1 &&
        res.out.at(0).indexOf("[ERROR]") != -1) {
      QString err_msg = res.out.at(0);
      CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
      res.res = SCWE_CANT_JOIN_SWARM;
    }

    if (res.exit_code != 0) {
      CApplicationLog::Instance()->LogError("Join to p2p swarm failed. Code : %d", res.exit_code);
      res.res = SCWE_CREATE_PROCESS;
    }

    QThread::currentThread()->msleep(500);
  } while (res.exit_code && --attempts_count);
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::leave_p2p_swarm(const QString& hash) {
  if (hash == NULL || !is_in_swarm(hash))
    return SCWE_SUCCESS;
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "stop" << "-hash" << hash;
  system_call_res_t res = ssystem_th(cmd, args, false); //we don't need output. AHAHA
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::restart_p2p_service(int *res_code) {
#if defined(RT_OS_LINUX)
  *res_code = RSE_MANUAL;
  return SCWE_SUCCESS;
#elif defined(RT_OS_WINDOWS)
  QString cmd("sc");
  QStringList args0, args1;
  args0 << "stop" << "\"Subutai Social P2P\"";
  args1 << "start" << "\"Subutai Social P2P\"";
  system_call_res_t res = ssystem_th(cmd, args0, false);
  res = ssystem_th(cmd, args1, false);
  *res_code = RSE_SUCCESS;
  return res.res;
#else
  QString cmd("osascript");
  QStringList args, lst_out;
  args << "-e" << "do shell script \"launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist;"
                  " launchctl load /Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
                  " with administrator privileges";
  int ec = 0;
  system_call_res_t res = ssystem_th(cmd, args, false);
  *res_code = RSE_SUCCESS;
  return res.res;
#endif
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::check_container_state(const QString& hash,
                                          const QString& ip) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show" << "-hash" << hash << "-check" << ip;
  system_call_res_t res = ssystem_th(cmd, args, false);
  return res.exit_code == 0 ? SCWE_SUCCESS : SCWE_CONTAINER_IS_NOT_READY;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_ssh_in_terminal(const QString& user,
                                        const QString& ip,
                                        const QString& port,
                                        const QString& key)
{
  QString str_command = QString("\"%1\" %2@%3 -p %4").
                        arg(CSettingsManager::Instance().ssh_path()).
                        arg(user).arg(ip).arg(port);

  if (!key.isEmpty()) {
    CNotificationObserver::Instance()->Info(QString("Using %1 ssh key").arg(key));
    str_command += QString(" -i \"%1\" ").arg(key);
  }

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res ;
    if ((tmp_res = which(CSettingsManager::Instance().terminal_cmd(), cmd)) != SCWE_SUCCESS) {
      return tmp_res;
    }
  }

  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args = CSettingsManager::Instance().terminal_arg().split(QRegularExpression("\\s"));

#ifdef RT_OS_DARWIN
  args << QString("Tell application \"Terminal\"\n"
                  "  Activate\n"
                  "  do script \""
                  "%1\"\n"
                  " end tell").arg(str_command);
#elif RT_OS_LINUX
  args << QString("%1;bash").arg(str_command);
#elif RT_OS_WINDOWS

  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  QString str_command_quoted = QString("%1").arg(str_command);
  QString cmd_args = QString("\"%1\" /k \"%2\"").arg(cmd).arg(str_command_quoted);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(NULL,
                          cmd_args_lpwstr,
                          NULL, NULL,
                          FALSE, 0,
                          NULL, NULL,
                          &si, &pi);
  if (!cp) {
    CApplicationLog::Instance()->LogError("Failed to create process %s. Err : %d",
                                          cmd.toStdString().c_str(), GetLastError());
    return SCWE_SSH_LAUNCH_FAILED;
  }
  return SCWE_SUCCESS;
#endif
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::generate_ssh_key(const QString &comment,
                                     const QString &file_path) {
  QString cmd = CSettingsManager::Instance().ssh_keygen_cmd();
  QStringList lst_args;
  lst_args << "-t" << "rsa" <<
              "-f" << file_path <<
              "-C" << comment <<
              "-N" << "";
  QStringList lst_out;
  system_call_res_t res = ssystem(cmd, lst_args, true);

  if (res.exit_code != 0 && res.res == SCWE_SUCCESS) {
    res.res = SCWE_CANT_GENERATE_SSH_KEY;
    CApplicationLog::Instance()->LogError("Can't generate ssh-key. exit_code : %d", res.exit_code);
  }

  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_libssh2_command(const char *host,
                                        uint16_t port,
                                        const char *user,
                                        const char *pass,
                                        const char *cmd,
                                        int& exit_code,
                                        std::vector<std::string>& lst_output) {
  static const int default_timeout = 10;
  exit_code = CLibsshController::run_ssh_command_pass_auth(host, port, user,
                                                           pass, cmd, default_timeout,
                                                           lst_output);

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t
CSystemCallWrapper::is_rh_update_available(bool &available) {
  available = false;
  int exit_code = 0;
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port(),
                          CSettingsManager::Instance().rh_user().toStdString().c_str(),
                          CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                          "sudo subutai update rh -c",
                          exit_code,
                          lst_out);
  if (res != SCWE_SUCCESS) return res;
  available = exit_code == 0;
  return SCWE_SUCCESS; //doesn't matter I guess.
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::is_rh_management_update_available(bool &available) {
  available = false;
  int exit_code = 0;
  std::vector<std::string> lst_out;

  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port(),
                          CSettingsManager::Instance().rh_user().toStdString().c_str(),
                          CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                          "sudo subutai update management -c",
                          exit_code,
                          lst_out);
  if (res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogError("is_rh_management_update_available failed with code %d", res);
    return res;
  }
  available = exit_code == 0;
  return SCWE_SUCCESS; //doesn't matter I guess.
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_rh_updater(const char *host,
                                   uint16_t port,
                                   const char *user,
                                   const char *pass,
                                   int& exit_code) {
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass, "sudo subutai update rh", exit_code, lst_out);
  return res;
}

system_call_wrapper_error_t
CSystemCallWrapper::run_rh_management_updater(const char *host,
                                              uint16_t port,
                                              const char *user,
                                              const char *pass,
                                              int &exit_code) {
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass, "sudo subutai update management", exit_code, lst_out);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::get_rh_ip_via_libssh2(const char *host,
                                          uint16_t port,
                                          const char *user,
                                          const char *pass,
                                          int &exit_code,
                                          std::string &ip) {
  system_call_wrapper_error_t res;
  std::vector<std::string> lst_out;
  QString rh_ip_cmd = QString("sudo subutai info ipaddr");
  res = run_libssh2_command(host, port, user, pass, rh_ip_cmd.toStdString().c_str(), exit_code, lst_out);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    QHostAddress addr(lst_out[0].c_str());
    if (!addr.isNull()) {
      ip = addr.toString().toStdString();
      return SCWE_SUCCESS;
    }
  }

  return SCWE_CANT_GET_RH_IP;
}
////////////////////////////////////////////////////////////////////////////

QString
CSystemCallWrapper::rh_version() {
  int exit_code;
  std::string version = "undefined";
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port(),
                          CSettingsManager::Instance().rh_user().toStdString().c_str(),
                          CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                          "sudo subutai -v",
                          exit_code,
                          lst_out);
  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0];

  size_t index ;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");

  return QString::fromStdString(version);
}
////////////////////////////////////////////////////////////////////////////

QString
CSystemCallWrapper::rhm_version() {
  int exit_code;
  std::string version = "undefined";
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port(),
                          CSettingsManager::Instance().rh_user().toStdString().c_str(),
                          CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                          "sudo subutai attach management grep git.build.version /opt/subutai-mng/etc/git.properties",
                          exit_code,
                          lst_out);
  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    for (auto str = lst_out.begin(); str != lst_out.end(); ++str) {
      if (str->find("git.build.version") == std::string::npos) continue;
      version = str->substr(str->find("=")+1);
    }
  }

  size_t index ;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");

  return QString::fromStdString(version);
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::p2p_version(QString &version) {
  version = "undefined";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "version";
  system_call_res_t res = ssystem_th(cmd, args, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty())
    version = res.out[0];
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::p2p_status(QString &status) {
  status = "";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "status";
  system_call_res_t res = ssystem_th(cmd, args, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    for (auto i = res.out.begin(); i != res.out.end(); ++i)
      status += *i;
  } else {
    status = "undefined";
  }
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::which(const QString &prog,
                          QString &path) {

  static int success_ec = 0;
#ifdef RT_OS_WINDOWS
  static const char* which_cmd = "where";
#else
  static const char* which_cmd = "which";
#endif
  QString cmd(which_cmd);
  QStringList args;
  args << prog;
  system_call_res_t res = ssystem_th(cmd, args, true, 5000);
  if (res.res != SCWE_SUCCESS) return res.res;

  if (res.exit_code == success_ec && !res.out.empty()) {
    path = res.out[0];
    path.replace("\n", "\0");
    path.replace("\r", "\0");
    return SCWE_SUCCESS;
  }

  return SCWE_WHICH_CALL_FAILED;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::chrome_version(QString &version) {
#if defined(RT_OS_LINUX)
  static const char* command= "/usr/bin/google-chrome-stable";
#elif defined(RT_OS_DARWIN)
  static const char* command = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome";
#elif defined(RT_OS_WINDOWS)
  static const char* command = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
  version = "Couldn't get version on Win, sorry";
  return SCWE_SUCCESS;
#endif
  version = "undefined";
  QString cmd(command);
  QStringList args, lst_out;
  args << "--version";

  system_call_res_t res = ssystem_th(cmd, args, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    version = res.out[0];
  }

  int index ;
  if ((index = version.indexOf('\n')) != -1)
    version.replace(index, 1, " ");
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

const QString&
CSystemCallWrapper::virtual_box_version() {
  return CVboxManager::Instance()->version();
}
////////////////////////////////////////////////////////////////////////////

const QString &
CSystemCallWrapper::scwe_error_to_str(system_call_wrapper_error_t err) {
  static QString error_str[] = {
    "SUCCESS", "Shell error", "Pipe error",
    "set_handle_info error", "create process error",
    "can't join to swarm", "container isn't ready",
    "ssh launch failed", "can't get rh ip address", "can't generate ssh-key",
    "call timeout", "which call failed", "process crashed"
  };
  return error_str[err];
}
////////////////////////////////////////////////////////////////////////////

void
CSystemCallThreadWrapper::do_system_call() {
  try {
    m_result = CSystemCallWrapper::ssystem(m_command, m_args, m_read_output);
  } catch (std::exception& exc) {
    CApplicationLog::Instance()->LogError("Err in CSystemCallThreadWrapper::do_system_call(). %s",
                                          exc.what());
    CApplicationLog::Instance()->LogError("Err in CSystemCallThreadWrapper::do_system_call(). %s",
                                          m_command.toStdString().c_str());
  }
  emit finished();
}
