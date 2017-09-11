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

#ifdef RT_OS_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#pragma comment(lib, "Advapi32.lib")
#endif

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

  if (!CCommons::IsApplicationLaunchable(CSettingsManager::Instance().p2p_path()))
    return SCWE_P2P_IS_NOT_RUNNABLE;

  CApplicationLog::Instance()->LogTrace("join to p2p swarm called. hash : %s",
                                        hash.toStdString().c_str());
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
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

template<class OS> system_call_wrapper_error_t
  restart_p2p_service_internal(int *res_code);

template<> system_call_wrapper_error_t
restart_p2p_service_internal<Os2Type<OS_LINUX> > (int *res_code) {
  *res_code = RSE_MANUAL;
  return SCWE_SUCCESS;
}
/*************************/

template<> system_call_wrapper_error_t
restart_p2p_service_internal<Os2Type<OS_WIN> >(int *res_code) {
  QString cmd("sc");
  QStringList args0, args1;
  args0 << "stop" << "\"Subutai Social P2P\"";
  args1 << "start" << "\"Subutai Social P2P\"";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, false);
  res = CSystemCallWrapper::ssystem_th(cmd, args1, false);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

template<> system_call_wrapper_error_t
restart_p2p_service_internal<Os2Type<OS_MAC> >(int *res_code) {
  QString cmd("osascript");
  QStringList args;
  args << "-e" << "do shell script \"launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist;"
                  " launchctl load /Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
                  " with administrator privileges";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, false);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

system_call_wrapper_error_t
CSystemCallWrapper::restart_p2p_service(int *res_code) {
  return restart_p2p_service_internal<Os2Type<CURRENT_OS> >( res_code);
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

template<class OS> system_call_wrapper_error_t run_ssh_in_terminal_internal(
    const QString& cmd,
    const QString& str_command);

template<> system_call_wrapper_error_t
run_ssh_in_terminal_internal<Os2Type<OS_LINUX> >(const QString& cmd,
                                                 const QString& str_command) {
  QStringList args = CSettingsManager::Instance().terminal_arg().split(QRegularExpression("\\s"));
  args << QString("%1;bash").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
}
/*********************/

template<> system_call_wrapper_error_t
run_ssh_in_terminal_internal<Os2Type<OS_MAC> >(const QString& cmd,
                                               const QString& str_command) {
  QStringList args = CSettingsManager::Instance().terminal_arg().split(QRegularExpression("\\s"));
  args << QString("Tell application \"Terminal\"\n"
                  "  Activate\n"
                  "  do script \""
                  "%1\"\n"
                  " end tell").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
}
/*********************/

template<> system_call_wrapper_error_t
run_ssh_in_terminal_internal<Os2Type<OS_WIN> >(const QString& cmd,
                                               const QString& str_command) {
  (void)cmd;
  (void)str_command; //make compiler happy.  %)
#ifdef RT_OS_WINDOWS
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  QString str_command_quoted = QString("%1").arg(str_command);
  QString cmd_args = QString("\"%1\" /k \"%2\"").arg(cmd).arg(str_command_quoted);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(NULL, cmd_args_lpwstr,
                          NULL, NULL,
                          FALSE, 0,
                          NULL, NULL,
                          &si, &pi);
  if (!cp) {
    CApplicationLog::Instance()->LogError("Failed to create process %s. Err : %d",
                                          cmd.toStdString().c_str(), GetLastError());
    return SCWE_SSH_LAUNCH_FAILED;
  }
#endif
  return SCWE_SUCCESS;
}
/*********************/

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
  return run_ssh_in_terminal_internal<Os2Type<CURRENT_OS> >(cmd, str_command);
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
  QString cmd(which_cmd());
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

template<class OS> system_call_wrapper_error_t
  chrome_version_internal(QString &version);

/********************/
template<> system_call_wrapper_error_t
chrome_version_internal<Os2Type<OS_MAC_LIN> >(QString& version) {
  version = "undefined";
  QStringList args, lst_out;
  args << "--version";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().chrome_path(), args,
                                     true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    version = res.out[0];
  }

  int index ;
  if ((index = version.indexOf('\n')) != -1)
    version.replace(index, 1, " ");
  return res.res;
}
/********************/

template<> system_call_wrapper_error_t
chrome_version_internal<Os2Type<OS_LINUX> >(QString& version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<> system_call_wrapper_error_t
chrome_version_internal<Os2Type<OS_MAC> >(QString& version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<> system_call_wrapper_error_t
chrome_version_internal<Os2Type<OS_WIN> >(QString& version) {
  version = "Couldn't get version on Win, sorry";
#if defined(RT_OS_WINDOWS)
  //todo implement  with reading registry value
#endif
  return SCWE_SUCCESS;
}

system_call_wrapper_error_t
CSystemCallWrapper::chrome_version(QString &version) {
  return chrome_version_internal<Os2Type<CURRENT_OS> >(version);
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
    "set_handle_info error", "create process error", "p2p is not installed or hasn't execute rights",
    "can't join to swarm", "container isn't ready",
    "ssh launch failed", "can't get rh ip address", "can't generate ssh-key",
    "call timeout", "which call failed", "process crashed"
  };
  return error_str[err];
}
////////////////////////////////////////////////////////////////////////////

static const QString APP_AUTOSTART_KEY = "subutai-tray";

template<class OS> bool set_application_autostart_internal(bool start);

template<> bool set_application_autostart_internal<Os2Type<OS_LINUX> >(bool start) {
  static const QString desktop_file_content_template = "[Desktop Entry]\n"
                                      "Type=Application\n"
                                      "Name=subutai-tray\n"
                                      "Exec=%1\n"
                                      "Hidden=false\n"
                                      "NoDisplay=false\n"
                                      "Comment=subutai software\n"
                                      "X-GNOME-Autostart-enabled=true\n";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    CApplicationLog::Instance()->LogError("Couldn't get standard locations. HOME");
    CNotificationObserver::Error("Couldn't get home directory, sorry");
    return false;
  }

  QString directory_path = lst_standard_locations[0] +
                           QDir::separator() +
                           ".config/autostart";
  QDir dir(directory_path);
  if (!dir.mkpath(directory_path)) {
    CApplicationLog::Instance()->LogError("Couldn't create autostart directory");
    CNotificationObserver::Error("Couldn't create autostart directory, sorry");
    return false;
  }

  QString desktop_file_path = directory_path +
                              QDir::separator() +
                              APP_AUTOSTART_KEY +
                              ".desktop";

  QFile desktop_file(desktop_file_path);

  if (!start) {
    if (!desktop_file.exists()) return true; //already removed from autostart.
    if (desktop_file.remove()) return true;
    CApplicationLog::Instance()->LogError("Couldn't delete file : %s",
                                          desktop_file.errorString().toStdString().c_str());
    CNotificationObserver::Error(QString("Couldn't delete %1. %2").
                                 arg(desktop_file_path).
                                 arg(desktop_file.errorString()));
    return false; //removed or not . who cares?
  }

  QString desktop_file_content = QString(desktop_file_content_template).
                                 arg(QApplication::applicationFilePath());
  if (!desktop_file.open(QFile::Truncate | QFile::WriteOnly)) {
    CApplicationLog::Instance()->LogError("Couldn't open desktop file for write");
    CNotificationObserver::Error(QString("Couldn't create autostart desktop file. Error : %1").
                                 arg(desktop_file.errorString()));
    return false;
  }

  bool result = true;
  do {
    QByteArray content_arr = desktop_file_content.toUtf8();
    if (desktop_file.write(content_arr) != content_arr.size()) {
      CApplicationLog::Instance()->LogError("Couldn't write content to autostart desktop file");
      CNotificationObserver::Error("Couldn't write content to autostart desktop file");
      result = false;
    }
  } while (0);

  desktop_file.close();
  return result;
}
/*********************/

template<> bool set_application_autostart_internal<Os2Type<OS_MAC> >(bool start) {

  static const QString item_location = "/Library/LaunchAgents/";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    CApplicationLog::Instance()->LogError("Couldn't get standard locations. HOME");
    CNotificationObserver::Error("Couldn't get home directory, sorry");
    return false;
  }

  QString item_path = lst_standard_locations[0] +
                      QDir::separator() +
                      item_location +
                      APP_AUTOSTART_KEY +
                      QString(".plist");


  QString content_template =
      QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
      "<plist version=\"1.0\">\n"
      "<dict>\n"
          "\t<key>Label</key>\n"
          "\t<string>%1</string>\n"
          "\t<key>ProgramArguments</key>\n"
              "\t\t<array>\n"
                  "\t\t\t<string>%2</string>\n"
              "\t\t</array>\n"
          "\t<key>RunAtLoad</key>\n"
          "\t<true/>\n"
      "</dict>\n"
      "</plist>\n").arg(APP_AUTOSTART_KEY).arg(QApplication::applicationFilePath());

  static const QString cmd("osascript");

  QFile item_file(item_path);
  if (!item_file.exists()) {
    if (!item_file.open(QFile::ReadWrite)) {
      CApplicationLog::Instance()->LogError("Couldn't create subutai-tray.plist file. Error : %s",
                                            item_file.errorString().toStdString().c_str());
      CNotificationObserver::Error(item_file.errorString());
      return false;
    }

    QByteArray content = content_template.toUtf8();
    if (item_file.write(content) != content.size()) {
      CApplicationLog::Instance()->LogError("Didn't write whole content to plist file");
      CNotificationObserver::Error("Write plist file error");
      item_file.close();
      return false;
    }
    item_file.close();
  }

  QStringList args;
  QString add_command = QString("launchctl load %1").arg(item_path);
  QString rem_command = QString("launchctl unload %1").arg(item_path);
  args << "-e" << QString("do shell script \"%1\"").arg(start ? add_command : rem_command);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, false);
  return res.res == SCWE_SUCCESS;
}
/*********************/

template<> bool set_application_autostart_internal<Os2Type<OS_WIN> >(bool start) {
  (void)start; //make compiler happy
  bool result = true;
#ifdef RT_OS_WINDOWS
  HKEY rkey_run = NULL;
  static const LPCWSTR val_name((wchar_t*)APP_AUTOSTART_KEY.utf16());
  DWORD disp;
  do { //try to write value to registry
    int32_t cr = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                 L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &rkey_run,
                                 &disp);

    if (cr != ERROR_SUCCESS || !rkey_run) {
      CApplicationLog::Instance()->LogError("Create registry key error. ec = %d, cr = %d",
                                            GetLastError(), cr);
      CNotificationObserver::Error("Couldn't create registry key, sorry");
      result = false;
      break;
    }

    if (start) {
      cr = RegSetKeyValueW(rkey_run,
                           0,
                           val_name,
                           REG_SZ,
                           QApplication::applicationFilePath().replace("/", "\\").utf16(),
                           QApplication::applicationFilePath().length()*2);

      if (cr == ERROR_ACCESS_DENIED) {
        CNotificationObserver::Error("Couldn't add program to autorun due to access denied. Try to run this application as administrator");
        result = false;
        break;
      }

      if (cr != ERROR_SUCCESS) {
        CApplicationLog::Instance()->LogError("RegSetKeyValue err : %d, %d",
                                              cr, GetLastError());
        CNotificationObserver::Error("Couldn't add program to autorun, sorry");
        result = false;
        break;
      }
    } else { //if (start)
      cr = RegDeleteKeyValueW(rkey_run,
                              0,
                              val_name);

      if (cr == ERROR_ACCESS_DENIED) {
        CNotificationObserver::Error("Couldn't remove program from autorun due to access denied. Try to run this application as administrator");
        result = false;
      }

      if (cr == ERROR_PATH_NOT_FOUND) {
        result = true;
        break;
      }

      if (cr != ERROR_SUCCESS) {
        CApplicationLog::Instance()->LogError("RegDeleteKeyValueW err : %d, %d",
                                              cr, GetLastError());
        CNotificationObserver::Error("Couldn't remove program from autorun, sorry");
        result = false;
        break;
      }
    } //if (start) ... else this block
  } while(0);

  RegCloseKey(rkey_run);
#endif
  return result;
}
/*********************/

bool
CSystemCallWrapper::set_application_autostart(bool start) {
  return set_application_autostart_internal<Os2Type<CURRENT_OS> >(start);
}
////////////////////////////////////////////////////////////////////////////

template<class OS> bool application_autostart_internal();

template<> bool application_autostart_internal<Os2Type<OS_LINUX> >() {
  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    CApplicationLog::Instance()->LogError("Couldn't get standard locations. HOME");
    return false;
  }

  QString directory_path = lst_standard_locations[0] +
                           QDir::separator() +
                           ".config/autostart";
  QString desktop_file_path = directory_path +
                              QDir::separator() +
                              APP_AUTOSTART_KEY +
                              ".desktop";

  QFile desktop_file(desktop_file_path);
  return desktop_file.exists(); //todo check exec field. should be QApplication::applicationFilePath()
}
/*********************/

template<> bool application_autostart_internal<Os2Type<OS_MAC> >() {

  static const QString item_location = "/Library/LaunchAgents/";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    CApplicationLog::Instance()->LogError("Couldn't get standard locations. HOME");
    return false;
  }

  QString item_path = lst_standard_locations[0] +
                      QDir::separator() +
                      item_location +
                      APP_AUTOSTART_KEY +
                      QString(".plist");

  QFile item_file(item_path);
  return item_file.exists(); //todo check first argument. should be QApplication::applicationFilePath()
}
/*********************/

template<> bool application_autostart_internal<Os2Type<OS_WIN> >() {
  bool result = true;
#ifdef RT_OS_WINDOWS
  HKEY rkey_run = NULL;
  do {
    static const LPCWSTR val_name((wchar_t*)APP_AUTOSTART_KEY.utf16());
    DWORD disp;
    int32_t cr = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                 L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &rkey_run,
                                 &disp);

    if (cr != ERROR_SUCCESS || !rkey_run) {
      CApplicationLog::Instance()->LogError("Create registry key error. ec = %d, cr = %d",
                                            GetLastError(), cr);
      result = false;
      break;
    }

    static const uint32_t buff_size = 1024;
    uint8_t buff[buff_size] = {0};
    DWORD cb_data;
    DWORD rr;
    rr = RegQueryValueEx( rkey_run,
                          val_name,
                          NULL,
                          NULL,
                          buff,
                          &cb_data );
    if (rr != ERROR_SUCCESS) {
      result = false;
      break;
    }

    QString qdata = QString::fromUtf16((ushort*)buff, cb_data);
    result = qdata == QApplication::applicationFilePath();
  } while (0);
  RegCloseKey(rkey_run);
#endif
  return result;
}
/*********************/

bool
CSystemCallWrapper::application_autostart() {
  return application_autostart_internal<Os2Type<CURRENT_OS> >();
}

CSystemCallWrapper::container_ip_and_port
CSystemCallWrapper::container_ip_address_from_subutai_list(
    const QString &cont_name,
    const QString &port,
    const QString &cont_ip,
    const QString &rh_ip) {

  QString cmd = subutai_list_command();
  QStringList args;
  args << "list" << "i";
  system_call_res_t scr = ssystem_th(cmd, args, true, 7000);
  CSystemCallWrapper::container_ip_and_port res;
  res.ip = QString(rh_ip);
  res.port = QString(port);

  if (scr.res == SCWE_SUCCESS && scr.exit_code == 0) {
    for (QString str : scr.out) {
      int ix1, ix2;
      if ((ix1 = str.indexOf(cont_name) == -1) ||
          (ix2 = str.indexOf(cont_ip) == -1)) continue;
      res.port = "22"; //MAGIC!!
      res.ip = QString(cont_ip);
    }
  }

  return res;
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
