#include <stdio.h>
#include <sstream>

#include <QHostAddress>
#include <QApplication>
#include <SystemCallWrapper.h>
#include "SettingsManager.h"
#include "NotifiactionObserver.h"
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>
#include "vbox/include/IVBoxManager.h"
#include "HubController.h"
#include "libssh2/include/LibsshController.h"

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#include <Process.h>
#endif
#include "ApplicationLog.h"

static QString error_strings[] = {
  "Success", "Shell error",
  "Pipe error", "Set handle info error",
  "Create process error", "Cant join to swarm",
  "SSH launch error", "System call timeout"
};

system_call_wrapper_error_t
CSystemCallWrapper::ssystem_th(const char *command,
                               std::vector<std::string> &lst_output,
                               int &exit_code,
                               bool read_output, unsigned long time_msec) {
  CSystemCallThreadWrapper sctw(command, read_output);
  QThread* th = new QThread;
  QObject::connect(&sctw, SIGNAL(finished()), th, SLOT(quit()), Qt::DirectConnection);
  QObject::connect(th, SIGNAL(started()), &sctw, SLOT(do_system_call()));
  QObject::connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));

  sctw.moveToThread(th);
  th->start();  
  if (!th->wait(time_msec)) {
    CApplicationLog::Instance()->LogError("Command %s failed with timeout.", command);
    return SCWE_TIMEOUT;
  }

  lst_output = std::move(sctw.lst_output());
  exit_code = sctw.exit_code();
  return sctw.result();
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::ssystem_th2(const QString &cmd,
                                const QStringList &args,
                                QStringList &lst_out,
                                int &exit_code,
                                bool read_output,
                                unsigned long timeout_msec) {
  CSystemCallThreadWrapper2 sctw(cmd, args, read_output);
  QThread* th = new QThread;
  QObject::connect(&sctw, SIGNAL(finished()), th, SLOT(quit()), Qt::DirectConnection);
  QObject::connect(th, SIGNAL(started()), &sctw, SLOT(do_system_call()));
  QObject::connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));

  sctw.moveToThread(th);
  th->start();
  if (!th->wait(timeout_msec)) {
    CApplicationLog::Instance()->LogError("Command %s failed with timeout.", cmd.toStdString().c_str());
    return SCWE_TIMEOUT;
  }

  lst_out = sctw.lst_output();
  exit_code = sctw.exit_code();
  return sctw.result();
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::ssystem(const char *command,
                            std::vector<std::string>& lst_output,
                            int &exit_code,
                            bool read_output) {
  CApplicationLog::Instance()->LogTrace("ssystem : %s", command);
#ifndef RT_OS_WINDOWS
  UNUSED_ARG(read_output);
  std::string str_cmd = " 2>&1 ";
  str_cmd = std::string(command) + str_cmd;
  FILE* pf = popen(str_cmd.c_str(), "r");
  if (pf) {
    char *line = NULL;
    size_t len = 0;
    int get_line_count = 0;
    while((get_line_count = getline(&line, &len, pf)) != -1 && !CCommons::QuitAppFlag) {
      lst_output.push_back(std::string(line, get_line_count));
      if (line) {free(line); line = NULL;}
    }
    if (line) free(line);

    if (CCommons::QuitAppFlag) return SCWE_SUCCESS;
    exit_code = pclose(pf);
    exit_code = WEXITSTATUS(exit_code);

    if (lst_output.empty())
      return SCWE_SHELL_ERROR;
  } else {
    CApplicationLog::Instance()->LogError("popen(%s) failed with code %d", str_cmd.c_str(), errno);
    return SCWE_PIPE;
  }
  return SCWE_SUCCESS;
#else
  HANDLE h_cso_r; //child std out read
  HANDLE h_cso_w; //child std out write pipes

  SECURITY_ATTRIBUTES sa_attrs;
  //system_call_error_t
  system_call_wrapper_error_t res = SCWE_SUCCESS;
  sa_attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa_attrs.bInheritHandle = TRUE;
  sa_attrs.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&h_cso_r, &h_cso_w, &sa_attrs, 0))
    return SCWE_PIPE;

  do {
    if (!SetHandleInformation(h_cso_r, HANDLE_FLAG_INHERIT, 0)) {
      res = SCWE_SET_HANDLE_INFO;
      break;
    }

    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    BOOL b_success = FALSE;
    ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &si, sizeof(STARTUPINFO) );
    si.cb = sizeof(STARTUPINFO);
    si.hStdError = h_cso_w;
    si.hStdOutput = h_cso_w;
    si.hStdInput = NULL;
    si.dwFlags |= STARTF_USESTDHANDLES;

    /*create child process*/
    char str_com[256] = {0};
    memcpy(str_com, command, strlen(command));
    b_success = CreateProcessA(
                  NULL,          // no module name. use command line
                  str_com,       // command line
                  NULL,          // process security attributes
                  NULL,          // primary thread security attributes
                  TRUE,          // handles are inherited
                  CREATE_NO_WINDOW,             // creation flags
                  NULL,          // use parent's environment
                  NULL,          // use parent's current directory
                  &si,           // STARTUPINFO pointer
                  &pi);          // receives PROCESS_INFORMATION

    if (!b_success) {
      res = SCWE_CREATE_PROCESS;
      break;
    } else {
      DWORD ec = 0;
      WaitForSingleObject(pi.hProcess, INFINITE);
      GetExitCodeProcess(pi.hProcess, &ec);
      exit_code = (int)ec;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }

    CApplicationLog::Instance()->LogTrace("read output = %d", read_output ? 1 : 0);

    if (read_output) {      
      DWORD dw_read;
      static const int BUFF_SIZE = 8*1024; //8kB
      char r_buff[BUFF_SIZE] = {0};
      b_success = FALSE;
      b_success = ReadFile( h_cso_r, r_buff, BUFF_SIZE, &dw_read, NULL);
      CApplicationLog::Instance()->LogTrace("ReadFile finished");
      if(!b_success || dw_read == 0) {
        break;
      }

      unsigned int f, l; //first last
      f = l = 0;
      for (l = 0; l < dw_read; ++l) {
        if (r_buff[l] != '\n') continue;
        lst_output.push_back(std::string(&r_buff[f], l-f));
        f = l+1;
      }
    }
  } while (false);

  CloseHandle(h_cso_r);
  CloseHandle(h_cso_w);
  return res;
#endif
}

system_call_wrapper_error_t
CSystemCallWrapper::ssystem2(const QString &cmd,
                             const QStringList &args,
                             QStringList &lst_output,
                             int &exit_code,
                             bool read_output) {
  QProcess proc;
  proc.start(cmd, args);

  if (!proc.waitForStarted()) {
    CApplicationLog::Instance()->LogError("Failed to wait for started process %s", cmd.toStdString().c_str());
    return SCWE_CREATE_PROCESS;
  }

  if (!proc.waitForFinished()) {
    proc.terminate();
    return SCWE_TIMEOUT;
  }

  if (read_output) {
    QString output = QString(proc.readAll());
    lst_output = output.split("\n", QString::SkipEmptyParts);
  }

  exit_code = proc.exitCode();
  system_call_wrapper_error_t res =
      proc.exitStatus() == QProcess::NormalExit ?
        SCWE_SUCCESS : SCWE_PROCESS_CRASHED;

  return res;
}
////////////////////////////////////////////////////////////////////////////

bool
CSystemCallWrapper::is_in_swarm(const char *hash) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "show";
  int exit_code = 0;
  system_call_wrapper_error_t res = ssystem_th2(cmd, args, lst_out, exit_code, true);

  if (res != SCWE_SUCCESS && exit_code != 1) {
    CNotificationObserver::NotifyAboutError(error_strings[res]);
    CApplicationLog::Instance()->LogError(error_strings[res].toStdString().c_str());
    return false;
  }

  //todo use indexOf
  bool is_in = false;
  for (auto i = lst_out.begin(); i != lst_out.end() && !is_in; ++i) {
    is_in |= (i->indexOf(hash) != -1);
  }
  return is_in;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::join_to_p2p_swarm(const char *hash,
                                      const char *key,
                                      const char *ip)
{
  if (is_in_swarm(hash))
    return SCWE_SUCCESS;

  CApplicationLog::Instance()->LogTrace("join to p2p swarm called. hash : %s", hash);
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "start" <<
          "-ip" << ip <<
          "-key" << key <<
          "-hash" << hash;

  system_call_wrapper_error_t res = SCWE_SUCCESS;
  int exit_code = 0;
  int attempts_count = 5;
  do {
    res = ssystem_th2(cmd, args, lst_out, exit_code, true);
    if (res != SCWE_SUCCESS)
      continue;
    if (lst_out.size() == 1 &&
        lst_out.at(0).indexOf("[ERROR]") != -1) {
      QString err_msg = lst_out.at(0);
      CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
      res = SCWE_CANT_JOIN_SWARM;
    }

    if (exit_code != 0) {
      CApplicationLog::Instance()->LogError("Join to p2p swarm failed. Code : %d", exit_code);
      res = SCWE_CREATE_PROCESS;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  } while (exit_code && --attempts_count);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::leave_p2p_swarm(const char *hash) {
  if (hash == NULL || !is_in_swarm(hash))
    return SCWE_SUCCESS;
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "stop" << "-hash" << hash;
  int exit_code = 0;
  system_call_wrapper_error_t res = ssystem_th2(cmd, args, lst_out, exit_code, true);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::restart_p2p_service(int *res_code) {
#if defined(RT_OS_LINUX)
  *res_code = RSE_MANUAL;
  return SCWE_SUCCESS;
#elif defined(RT_OS_WINDOWS)
  QString cmd("sc");
  QStringList args0, args1, lst_out0, lst_out1;
  args0 << "stop" << "\"Subutai Social P2P\"";
  args1 << "start" << "\"Subutai Social P2P\"";
  int ec = 0;
  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args0, lst_out0, ec, true);
  res = ssystem_th2(cmd, args1, lst_out1, ec, true);
  *res_code = RSE_SUCCESS;
  return res;
#else
  QString cmd("osascript");
  QStringList args, lst_out;
  args << "-e" << "'do shell script \"launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist;"
                  " launchctl load /Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
                  " with administrator privileges'";
  int ec = 0;
  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args, lst_out, ec, true);
  *res = RSE_SUCCESS;
  return res;
#endif
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::check_container_state(const char *hash,
                                          const char *ip) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "show" << "-hash" << hash << "-check" << ip;
  int exit_code = 0;
  ssystem_th2(cmd, args, lst_out, exit_code, false);
  return exit_code == 0 ? SCWE_SUCCESS : SCWE_CONTAINER_IS_NOT_READY;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_ssh_in_terminal(const QString& user,
                                        const QString& ip,
                                        const QString& port,
                                        const QString& key)
{
  QString str_command = QString("%1 %2@%3 -p %4").
                        arg(CSettingsManager::Instance().ssh_path()).
                        arg(user).arg(ip).arg(port);

  if (key != NULL) {
    CNotificationObserver::Instance()->NotifyAboutInfo(QString("Using %1 ssh key").arg(key));
    str_command += QString(" -i \'%1\' ").arg(key);
  }

#ifdef RT_OS_DARWIN
  QString cmd("osascript");
  QStringList args;
  args << "-e" <<
          QString("\'Tell application \"Terminal\"\n"
                  "  Activate\n"
                  "  do script \""
                  "%1\"\n"
                  " end tell\'").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
#elif RT_OS_LINUX
  QString cmd("xterm");
  QStringList args;
  args << "-e" << QString("%1;bash").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
#elif RT_OS_WINDOWS
  QString cmd("cmd");
  QStringList args;
  args << "/k" << str_command;
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS : SCWE_SSH_LAUNCH_FAILED;
#endif
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::generate_ssh_key(const QString &comment,
                                     const QString &file_path) {
  QString cmd("ssh-keygen");
  QStringList lst_args;
  lst_args << "-t" << "rsa" <<
              "-f" << file_path <<
              "-C" << comment;// <<
//              "-N" << "\'\'";
  QStringList lst_out;
  int ec;
  system_call_wrapper_error_t res =
      ssystem2(cmd, lst_args, lst_out, ec, true);

  if (ec != 0 && res == SCWE_SUCCESS) {
    res = SCWE_CANT_GENERATE_SSH_KEY;
    CApplicationLog::Instance()->LogError("Can't generate ssh-key. exit_code : %d", ec);
  }

  return res;
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
CSystemCallWrapper::run_ss_updater(const char *host,
                                   uint16_t port,
                                   const char *user,
                                   const char *pass,
                                   int& exit_code) {
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass, "sudo subutai update rh", exit_code, lst_out);
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

system_call_wrapper_error_t
CSystemCallWrapper::p2p_version(std::string &version) {
  int exit_code;
  version = "undefined";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "version";
  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args, lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0].toStdString();
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::p2p_status(std::string &status) {
  int exit_code;
  status = "";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args, lst_out;
  args << "status";
  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args, lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    for (auto i = lst_out.begin(); i != lst_out.end(); ++i) {
      status += (*i).toStdString();
    }
  }
  else {
    status = "undefined";
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::which(const QString &prog,
                          std::string &path) {

  static int success_ec = 0;
#ifdef RT_OS_WINDOWS
  static const char* which_cmd = "where";
#else
  static const char* which_cmd = "which";
#endif
  QString cmd(which_cmd);
  QStringList args, lst_out;
  args << prog;
  int exit_code;
  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args, lst_out, exit_code, true);
  if (res != SCWE_SUCCESS) return res;

  if (exit_code == success_ec && !lst_out.empty()) {
    path = lst_out[0].toStdString();
    return SCWE_SUCCESS;
//    size_t index ;
//    if ((index = path.find('\n')) != std::string::npos)
//      path.replace(index, 1, "\0");
//    if ((index = path.find('\r')) != std::string::npos)
//      path.replace(index, 1, "\0");
  }

  return SCWE_WHICH_CALL_FAILED;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::chrome_version(std::string &version) {
#if defined(RT_OS_LINUX)
  static const char* command= "/usr/bin/google-chrome-stable";
#elif defined(RT_OS_DARWIN)
  static const char* command = "/Applications/Google\\ Chrome.app/Contents/MacOS/Google\\ Chrome";
#elif defined(RT_OS_WINDOWS)
  static const char* command = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
  version = "Couldn't get version on Win, sorry";
  return SCWE_SUCCESS;
#endif
  int exit_code;
  version = "undefined";
  QString cmd(command);
  QStringList args, lst_out;
  args << "--version";

  system_call_wrapper_error_t res =
      ssystem_th2(cmd, args, lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0].toStdString();

  size_t index ;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");
  return res;
}
////////////////////////////////////////////////////////////////////////////

QString
CSystemCallWrapper::virtual_box_version() {
  return CVBoxManagerSingleton::Instance()->version();
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
  m_result = CSystemCallWrapper::ssystem(m_command.c_str(), m_lst_output, m_exit_code, m_read_output);
  emit finished();
}
////////////////////////////////////////////////////////////////////////////

void
CSystemCallThreadWrapper2::do_system_call() {
  m_result = CSystemCallWrapper::ssystem2(m_command, m_args, m_lst_output, m_exit_code, m_read_output);
  emit finished();
}
