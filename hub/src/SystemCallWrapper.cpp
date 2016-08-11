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
#include <thread>

#include "HubController.h"

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
////////////////////////////////////////////////////////////////////////////

bool
CSystemCallWrapper::is_in_swarm(const char *hash) {
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " show";
  std::string command = str_stream.str();

  std::vector<std::string> lst_out;
  int exit_code = 0;

  CApplicationLog::Instance()->LogTrace("is in swarm called with hash : %s", hash);
  system_call_wrapper_error_t res = ssystem_th(command.c_str(), lst_out, exit_code, true);
  CApplicationLog::Instance()->LogTrace("ssystem_th end with result : %d", (int)res);

  if (res != SCWE_SUCCESS && exit_code != 1) {
    CNotificationObserver::NotifyAboutError(error_strings[res]);
    CApplicationLog::Instance()->LogError(error_strings[res].toStdString().c_str());
    return false;
  }

  bool is_in = false;
  for (auto i = lst_out.begin(); i != lst_out.end() && !is_in; ++i) {
    is_in |= (i->find(hash) != std::string::npos);
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
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash;
  std::string command = str_stream.str();
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res = SCWE_SUCCESS;
  int exit_code = 0;
  int attempts_count = 5;
  do {
    res = ssystem_th(command.c_str(), lst_out, exit_code, true);
    CApplicationLog::Instance()->LogTrace("ssystem_th ended with code : %d", (int)res);
    if (res != SCWE_SUCCESS) {
      continue;
    }

    if (lst_out.size() == 1 &&
        lst_out[0].find("[ERROR]") != std::string::npos) {
      QString err_msg = QString::fromStdString(lst_out[0]);
      CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
      res = SCWE_CANT_JOIN_SWARM;
    }

    if (exit_code != 0) {
      QString err_msg = QString("Join to p2p swarm failed. Code : %1").arg(exit_code);
      CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
      res = SCWE_CREATE_PROCESS;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  } while (exit_code != 0 && --attempts_count > 0);

  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::leave_p2p_swarm(const char *hash) {
  if (hash == NULL) return SCWE_SUCCESS;
  if (!is_in_swarm(hash))
    return SCWE_SUCCESS;
  CApplicationLog::Instance()->LogTrace("leave p2p swarm called. hash : %s", hash);
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " stop -hash " << hash;
  std::string command = str_stream.str();
  std::vector<std::string> lst_out;
  int exit_code = 0;
  system_call_wrapper_error_t res = ssystem_th(command.c_str(), lst_out, exit_code, true);
  CApplicationLog::Instance()->LogTrace("ssystem_th ended with code : %d", (int)res);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::check_container_state(const char *hash,
                                          const char *ip) {
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " show -hash " <<
                hash << " -check " << ip;
  std::vector<std::string> lst_out;
  int exit_code = 0;
  std::string command = str_stream.str();
  ssystem_th(command.c_str(), lst_out, exit_code, false);
  return exit_code == 0 ? SCWE_SUCCESS : SCWE_CONTAINER_IS_NOT_READY;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_ssh_in_terminal(const char* user,
                                        const char* ip,
                                        const char *port,
                                        const char *key)
{
  std::ostringstream str_stream;
  std::string str_command = CSettingsManager::Instance().ssh_path().toStdString().c_str() +
                            std::string(" ") +
                            std::string(user) +
                            std::string("@") +
                            std::string(ip) +
                            std::string(" -p ") +
                            std::string(port);

  if (key != NULL) {
    CApplicationLog::Instance()->LogTrace("KEY != NULL : %s", key);
    str_command += std::string(" -i \'") +
                   std::string(key) +
                   std::string("\' ");
  }
  CApplicationLog::Instance()->LogTrace("run ssh in terminal : %s", str_command.c_str());
#ifdef RT_OS_DARWIN
  str_stream << "osascript -e \'Tell application \"Terminal\"\n" <<
                "  Activate\n" <<
                "  do script \"" <<
                str_command.c_str() << "\"\n" <<
                " end tell\'";
  CApplicationLog::Instance()->LogTrace("OSX ssh->container command : %s", str_stream.str().c_str());
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_LINUX
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " -e \"" << str_command.c_str() << ";bash\" &";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_WINDOWS
  str_stream <<
             CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
             " /k " << str_command.c_str();
  PROCESS_INFORMATION pi;
  STARTUPINFOA si;
  ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
  ZeroMemory( &si, sizeof(STARTUPINFO) );
  si.cb = sizeof(STARTUPINFO);
  si.lpTitle = (LPSTR)"Subutai SSH";

  char str_com[512] = {0};
  memcpy(str_com, str_stream.str().c_str(), str_stream.str().size());

  CApplicationLog::Instance()->LogTrace("Run ssh->container command : %s", str_stream.str().c_str());
  BOOL success = CreateProcessA(NULL,
                                str_com,
                                NULL,
                                NULL,
                                FALSE,
                                CREATE_NEW_CONSOLE,
                                NULL,
                                NULL,
                                &si,
                                &pi);
  return success ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;

#endif
}
////////////////////////////////////////////////////////////////////////////

#include <QDebug>
system_call_wrapper_error_t
CSystemCallWrapper::generate_ssh_key(const char *comment,
                                     const char *file_path) {
  std::string key_str(file_path);
  std::string str_command = CSettingsManager::Instance().ssh_keygen_path().toStdString() +
                            std::string(" -t rsa") +
                            std::string(" -f ") + key_str +
                            std::string(" -C ") + std::string(comment) +
                            std::string(" -N \'\'");
  std::vector<std::string> lst_out;
  int exit_code;
  system_call_wrapper_error_t res = ssystem_th(str_command.c_str(), lst_out, exit_code, true);
  if (exit_code != 0 && res == SCWE_SUCCESS) {
    res = SCWE_CANT_GENERATE_SSH_KEY;
    CApplicationLog::Instance()->LogError("Can't generate ssh-key. exit_code : %d", exit_code);
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_libssh2_command(const char *host,
                                        const char *port,
                                        const char *user,
                                        const char *pass,
                                        const char *cmd,
                                        int& exit_code,
                                        std::vector<std::string>& lst_output) {
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().libssh2_app_path().toStdString().c_str() << " \"" << host << "\"" <<
                " \"" << port << "\"" << " \"" << user << "\"" << " \"" << pass << "\"" << " \"" << cmd << "\"";
  system_call_wrapper_error_t res =
      ssystem_th(str_stream.str().c_str(), lst_output, exit_code, true);
  return res;
}

system_call_wrapper_error_t
CSystemCallWrapper::is_rh_update_available(bool &available) {
  available = false;
  int exit_code = 0;
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port().toStdString().c_str(),
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
                                   const char *port,
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
                                          const char *port,
                                          const char *user,
                                          const char *pass,
                                          int &exit_code,
                                          std::string &ip) {
  system_call_wrapper_error_t res;
  std::vector<std::string> lst_out;
  QString rh_ip_cmd = QString("sudo subutai management_network detect");
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

system_call_wrapper_error_t
CSystemCallWrapper::fork_process(const QString program,
                                 const QStringList argv,
                                 const QString& folder) {

  QObject *parent = new QObject;
  QProcess *p = new QProcess(parent);
  system_call_wrapper_error_t res = p->startDetached(program, argv, folder) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
  delete p;
  return res;
}

////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t
CSystemCallWrapper::open_url(QString s_url) {
  QUrl q_url =QUrl(s_url);
  return QDesktopServices::openUrl(q_url) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
}
////////////////////////////////////////////////////////////////////////////

QString
CSystemCallWrapper::rh_version() {
  int exit_code;
  std::string version = "undefined";
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                          CSettingsManager::Instance().rh_port().toStdString().c_str(),
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
  std::vector<std::string> lst_out;
  std::string command = CSettingsManager::Instance().p2p_path().toStdString();
  command += std::string(" version");
  system_call_wrapper_error_t res =
      ssystem_th(command.c_str(), lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0];

  size_t index ;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::p2p_status(std::string &status) {
  int exit_code;
  status = "";
  std::vector<std::string> lst_out;
  std::string command = CSettingsManager::Instance().p2p_path().toStdString();
  command += std::string(" status");
  system_call_wrapper_error_t res =
      ssystem_th(command.c_str(), lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    for (auto i = lst_out.begin(); i != lst_out.end(); ++i) {
      status += *i;
    }
  }
  else
    status = "undefined";

  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::which(const std::string &prog,
                          std::string &path) {

#ifdef RT_OS_WINDOWS
  static const char* which_cmd = "where";
  static int success_ec = 0;
#else
  static const char* which_cmd = "which";
  static int success_ec = 1;
#endif
  std::vector<std::string> lst_out;
  std::string command(which_cmd);
  command += std::string(" ") + prog;
  int exit_code;
  system_call_wrapper_error_t res =
      ssystem_th(command.c_str(), lst_out, exit_code, true);

  if (res != SCWE_SUCCESS) return res;

  if (exit_code == success_ec && !lst_out.empty()) {
    path = lst_out[0];
    return SCWE_SUCCESS;
  }

  return SCWE_WHICH_CALL_FAILED;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::chrome_version(std::string &version) {
  int exit_code;
  version = "undefined";
  std::vector<std::string> lst_out;
  std::string command;

#if defined(RT_OS_LINUX)
  command = "/usr/bin/google-chrome-stable";
#elif defined(RT_OS_DARWIN)
  command = "/Applications/Google\\ Chrome.app/Contents/MacOS/Google\\ Chrome";
#elif defined(RT_OS_WINDOWS)
  command = "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe";
  version = "Couldn't get version on Win, sorry";
  return SCWE_SUCCESS;
#endif

  command += std::string(" --version");

  system_call_wrapper_error_t res =
      ssystem_th(command.c_str(), lst_out, exit_code, true);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0];

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
    "call timeout", "which call failed"
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
