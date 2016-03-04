
#include <sstream>
#include <stdio.h>
#include <iostream>

#include "SystemCallWrapper.h"
#include "SettingsManager.h"

#include <QDebug>

//check if it is crossplatform. should be because of using std.
std::vector<std::string> CSystemCallWrapper::ssystem(const char *command) {
  std::vector<std::string> res;
  FILE* pf = popen(command, "r");
  if (pf) {
    char * line = NULL;
    size_t len = 0;

    while(getline(&line, &len, pf) != -1) {
      qDebug() << line;
      res.push_back(std::string(line, len));
    }
    pclose(pf);
  }
  return res;
}
////////////////////////////////////////////////////////////////////////////

#include <QDebug>
bool CSystemCallWrapper::join_to_p2p_swarm(const char *hash,
                                           const char *key,
                                           const char *ip)
{
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash;
#ifndef RT_OS_WINDOWS
  str_stream << " &";
#endif
  std::string command = str_stream.str();
  qDebug() << command.c_str();
  std::vector<std::string> res = ssystem(command.c_str());

  for (auto i = res.begin(); i != res.end(); ++i)
    qDebug() << i->c_str();

  return true; //TODO CHECK RESULT!!!!
}
////////////////////////////////////////////////////////////////////////////

std::vector<std::string> CSystemCallWrapper::p2p_swarms_presented()
{
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " show";
  std::string command = str_stream.str();
  return ssystem(command.c_str());
}
////////////////////////////////////////////////////////////////////////////

void CSystemCallWrapper::run_ssh_in_terminal(const char* user,
                                             const char* ip)
{
  std::ostringstream str_stream;
#ifdef RT_OS_DARWIN
  str_stream << "osascript -e \'Tell application \"Terminal\" to do script \"" <<
                "ssh " << user << "@" << ip << "\"\'";
#elif RT_OS_LINUX
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " -e \"ssh " << user << "@" << ip << "\" &";
#elif RT_OS_WINDOWS
  str_stream <<
                CSettingsManager::Instance().terminal_path().toStdString().c_str() <<
                " /k ssh " << user << "@" << ip;
#endif
  ssystem(str_stream.str().c_str());
}
////////////////////////////////////////////////////////////////////////////
