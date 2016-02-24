
#include <iostream>
#include <fstream>
#include <sstream>
#include "SystemCallWrapper.h"
#include "SettingsManager.h"

//check if it is crossplatform. should be because of using std.
std::vector<std::string> CSystemCallWrapper::ssystem(const char *command) {
  std::vector<std::string> res;
  //create temp file
  char tmpname[L_tmpnam];
  std::tmpnam(tmpname);

  //run command > temp file
  std::string scommand = command;
  std::string cmd = scommand + " >> " + tmpname;
  std::system(cmd.c_str());

  //collect results from tmp file
  std::ifstream file(tmpname, std::ios::in);

  if (file) {
    while (!file.eof()) {
      std::string line;
      file >> line;
      res.push_back(line);
    }
    file.close();
  }

  //remove tmp file
  std::remove(tmpname);
  return res;
}
////////////////////////////////////////////////////////////////////////////


std::vector<std::string> CSystemCallWrapper::join_to_p2p_swarm(const char *hash,
                                                               const char *key,
                                                               const char *ip)
{
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().p2p_path().toStdString() << " start -ip " <<
                ip << " -key " << key << " -hash " << hash << " &"; //todo CHECK & !!!!!!!!!!!!!!
  std::string command = str_stream.str();
  return ssystem(command.c_str());
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

void CSystemCallWrapper::run_terminal()
{
  std::ostringstream str_stream;
  str_stream << CSettingsManager::Instance().terminal_path().toStdString().c_str() << " &";
  ssystem(str_stream.str().c_str());
}
////////////////////////////////////////////////////////////////////////////
