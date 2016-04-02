#include <stdio.h>
#include <sstream>
#include <QDebug>

#include <SystemCallWrapper.h>
#include "SettingsManager.h"
#include "NotifiactionObserver.h"
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#include <Process.h>
#endif


static QString error_strings[] = {
  "Success", "Shell error",
  "Pipe error", "Set handle info error",
  "Create process error", "Cant join to swarm",
  "SSH launch error"
};

/*
 * On Windows we can't launch long time processes by calling this method.
 * BE CAREFUL!!! ATENTION!!! WARNING!!! ACHTUNG!!!!
*/

system_call_wrapper_error_t
CSystemCallWrapper::ssystem(const char *command,
                            std::vector<std::string>& lst_output, int &exit_code) {
#ifndef RT_OS_WINDOWS
  FILE* pf = popen(command, "r");
  if (pf) {
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, pf) != -1)
      lst_output.push_back(std::string(line, len));
    exit_code = pclose(pf);
    exit_code = WEXITSTATUS(exit_code);

    if (lst_output.empty())
      return SCWE_SHELL_ERROR;
  } else {
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

    /*read from file*/
    DWORD dw_read;
    char r_buff[4096] = {0};
    b_success = FALSE;
    b_success = ReadFile( h_cso_r, r_buff, 256, &dw_read, NULL);
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
  system_call_wrapper_error_t res = ssystem(command.c_str(), lst_out, exit_code);
  if (res != SCWE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutError(error_strings[res]);
    qCritical() << error_strings[res];
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

  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash;
  std::string command = str_stream.str();
  std::vector<std::string> lst_out;
  int exit_code;
  system_call_wrapper_error_t res = ssystem(command.c_str(), lst_out, exit_code);
  if (res != SCWE_SUCCESS) {
    QString err_msg = QString("Join to p2p failed. Error : %1").arg(res);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    qCritical() << err_msg;
    return res;
  }

  if (lst_out.size() == 1 &&
      lst_out[0].find("[ERROR]") != std::string::npos) {
    QString err_msg = QString::fromStdString(lst_out[0]);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    qCritical() << err_msg;
    return SCWE_CANT_JOIN_SWARM;
  }

  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_ssh_in_terminal(const char* user,
                                        const char* ip,
                                        const char *port)
{
  std::ostringstream str_stream;
#ifdef RT_OS_DARWIN
  str_stream << "osascript -e \'Tell application \"Terminal\"\n" <<
                "  Activate\n" <<
                "  do script \"" <<
                "ssh " << user << "@" << ip << " -p " << port << "\"\n" <<
                " end tell\'";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_LINUX
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " -e \"ssh " << " " << user << "@" << ip << " -p " << port << ";bash\" &";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_WINDOWS
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " /k ssh " << user << "@" << ip << " -p " << port;
  PROCESS_INFORMATION pi;
  STARTUPINFOA si;
  ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
  ZeroMemory( &si, sizeof(STARTUPINFO) );
  si.cb = sizeof(STARTUPINFO);
  si.lpTitle = (LPSTR)"Subutai SSH";

  char str_com[256] = {0};
  memcpy(str_com, str_stream.str().c_str(), str_stream.str().size());

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

system_call_wrapper_error_t
CSystemCallWrapper::run_ss_updater(const char *host,
                                   const char *port,
                                   const char *user,
                                   const char *pass,
                                   const char *cmd,
                                   int& exit_code) {
  std::ostringstream str_stream;
  std::vector<std::string> lst_out;
  str_stream << CSettingsManager::Instance().ss_updater_path().toStdString().c_str() << " \"" << host << "\"" <<
                " \"" << port << "\"" << " \"" << user << "\"" << " \"" << pass << "\"" << " \"" << cmd << "\"";
  system_call_wrapper_error_t res =
      ssystem(str_stream.str().c_str(), lst_out, exit_code);
  //todo check res and lst_out
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::fork_process(const QString program,
                                 const QStringList argv,
                                 const QString& folder){

  QObject *parent = new QObject;
  QProcess *p = new QProcess(parent);
  return p->startDetached(program, argv, folder) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
}

////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t
CSystemCallWrapper::open_url(QString s_url){
  QUrl q_url =QUrl(s_url);
  return QDesktopServices::openUrl(q_url) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
}
////////////////////////////////////////////////////////////////////////////
