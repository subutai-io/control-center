
#include <stdio.h>
#include <sstream>

#include "SystemCallWrapper.h"
#include "SettingsManager.h"

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#include <Process.h>
#endif

#include <QDebug>

/*
 * On Windows we can't launch long time processes by calling this method.
 * BE CAREFUL!!! ATENTION!!! WARNING!!! ACHTUNG!!!!
*/

#pragma message ("Read comment to this function. Don't use this function for launching long time processes")

system_call_wrapper_error_t
CSystemCallWrapper::ssystem(const char *command,
                            std::vector<std::string>& lst_output) {
#ifndef RT_OS_WINDOWS
  FILE* pf = popen(command, "r");
  if (pf) {
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, pf) != -1)
      lst_output.push_back(std::string(line, len));
    pclose(pf);

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
  system_call_error_t res = SCE_SUCCESS;
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
      WaitForSingleObject(pi.hProcess, INFINITE);
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
  system_call_wrapper_error_t res = ssystem(command.c_str(), lst_out);
  if (res != SCWE_SUCCESS) {
    //todo log error
    return res;
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
  if (is_in_swarm(hash)) return SCWE_SUCCESS;

  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash;
  std::string command = str_stream.str();
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res = ssystem(command.c_str(), lst_out);
  if (res != SCWE_SUCCESS)
    return res;

  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::run_ssh_in_terminal(const char* user,
                                        const char* ip)
{
  std::ostringstream str_stream;
#ifdef RT_OS_DARWIN
  str_stream << "osascript -e \'Tell application \"Terminal\" to do script \"" <<
                "ssh " << user << "@" << ip << "\"\'";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_LINUX
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " -e \"ssh " << user << "@" << ip << ";bash\" &";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_WINDOWS
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " /k ssh " << user << "@" << ip;
  PROCESS_INFORMATION pi;
  STARTUPINFOA si;
  ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
  ZeroMemory( &si, sizeof(STARTUPINFO) );
  si.cb = sizeof(STARTUPINFO);
  si.lpTitle = (LPSTR)"Subutai SSH";

  char str_com[256] = {0};
  memcpy(str_com, str_stream.str().c_str(), str_stream.str().size());

  CreateProcessA(NULL,
                 str_com,
                 NULL,
                 NULL,
                 FALSE,
                 CREATE_NEW_CONSOLE,
                 NULL,
                 NULL,
                 &si,
                 &pi);
  return SCWE_SUCCESS;

#endif
}
////////////////////////////////////////////////////////////////////////////
