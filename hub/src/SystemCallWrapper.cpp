#include <stdio.h>
#include <sstream>

#include <QApplication>
#include <SystemCallWrapper.h>
#include "SettingsManager.h"
#include "NotifiactionObserver.h"
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#include <Process.h>
#endif
#include "ApplicationLog.h"

static QString error_strings[] = {
  "Success", "Shell error",
  "Pipe error", "Set handle info error",
  "Create process error", "Cant join to swarm",
  "SSH launch error"
};

system_call_wrapper_error_t CSystemCallWrapper::ssystem_th(const char *command,
                                                           std::vector<std::string> &lst_output,
                                                           int &exit_code) {
  QEventLoop el;
  CSystemCallThreadWrapper sctw(command);
  QThread* th = new QThread;

  QObject::connect(&sctw, SIGNAL(finished()), &el, SLOT(quit()));
  QObject::connect(th, SIGNAL(started()), &sctw, SLOT(do_system_call()));
  QObject::connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));
  sctw.moveToThread(th);
  th->start();
  el.exec();

  lst_output = std::vector<std::string>(sctw.lst_output());
  exit_code = sctw.exit_code();
  return sctw.result();
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::ssystem(const char *command,
                            std::vector<std::string>& lst_output, int &exit_code) {
#ifndef RT_OS_WINDOWS
  std::string str_cmd = " 2>&1 ";
  str_cmd = std::string(command) + str_cmd;
  FILE* pf = popen(str_cmd.c_str(), "r");
  if (pf) {
    char *line = NULL;
    size_t len = 0;

    while(getline(&line, &len, pf) != -1 && !CCommons::QuitAppFlag) {
      lst_output.push_back(std::string(line, len));
    }

    if (CCommons::QuitAppFlag) return SCWE_SUCCESS;
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

  system_call_wrapper_error_t res = ssystem_th(command.c_str(), lst_out, exit_code);
  if (res != SCWE_SUCCESS && exit_code != 1) {
    CNotifiactionObserver::NotifyAboutError(error_strings[res]);
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

  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash;
  std::string command = str_stream.str();
  std::vector<std::string> lst_out;
  int exit_code = 0;
  system_call_wrapper_error_t res = ssystem_th(command.c_str(), lst_out, exit_code);
  if (res != SCWE_SUCCESS) {
    QString err_msg = QString("Join to p2p failed. Error : %1").arg(res);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
    return res;
  }

  if (lst_out.size() == 1 &&
      lst_out[0].find("[ERROR]") != std::string::npos) {
    QString err_msg = QString::fromStdString(lst_out[0]);
    CNotifiactionObserver::NotifyAboutError(err_msg);
    CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
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
                CSettingsManager::Instance().ssh_path().toStdString().c_str() <<
                " " << user << "@" << ip << " -p " << port << "\"\n" <<
                " end tell\'";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_LINUX
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " -e \"" << CSettingsManager::Instance().ssh_path().toStdString().c_str() <<
                " " << user << "@" << ip << " -p " << port << ";bash\" &";
  return system(str_stream.str().c_str()) == -1 ? SCWE_SSH_LAUNCH_FAILED : SCWE_SUCCESS;
#elif RT_OS_WINDOWS
  str_stream <<
             CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
             " /k " << CSettingsManager::Instance().ssh_path().toStdString().c_str() << " "
             << user << "@" << ip << " -p " << port;
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
CSystemCallWrapper::run_libssh2_command(const char *host,
                                        const char *port,
                                        const char *user,
                                        const char *pass,
                                        const char *cmd,
                                        int& exit_code,
                                        std::vector<std::string>& lst_output) {
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().ss_updater_path().toStdString().c_str() << " \"" << host << "\"" <<
                " \"" << port << "\"" << " \"" << user << "\"" << " \"" << pass << "\"" << " \"" << cmd << "\"";
  qDebug() << str_stream.str().c_str();
  system_call_wrapper_error_t res =
      ssystem_th(str_stream.str().c_str(), lst_output, exit_code);
  return res;
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
      run_libssh2_command(host, port, user, pass, "subutai-update", exit_code, lst_out);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::get_rh_ip_via_libssh2(const char *host,
                                                                      const char *port,
                                                                      const char *user,
                                                                      const char *pass,
                                                                      int &exit_code,
                                                                      std::string &ip)
{
  static const char* rh_ip_cmd = "ifconfig wan | grep 'inet addr:' | cut -d: -f2| cut -d ' ' -f 1";
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass, rh_ip_cmd, exit_code, lst_out);
  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    ip = lst_out[0];
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::fork_process(const QString program,
                                 const QStringList argv,
                                 const QString& folder){

  QObject *parent = new QObject;
  QProcess *p = new QProcess(parent);
  system_call_wrapper_error_t res = p->startDetached(program, argv, folder) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
  delete p;
  return res;
}

////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t
CSystemCallWrapper::open_url(QString s_url){
  QUrl q_url =QUrl(s_url);
  return QDesktopServices::openUrl(q_url) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
}
////////////////////////////////////////////////////////////////////////////

void CSystemCallThreadWrapper::do_system_call()
{
  m_result = CSystemCallWrapper::ssystem(m_command.c_str(), m_lst_output, m_exit_code);
  emit finished();
}
////////////////////////////////////////////////////////////////////////////
