#include "SystemCallWrapper.h"

#include <iostream>
#include <fstream>
#include <sstream>

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
                                                               const char *ip,
                                                               const char *p2p)
{
  std::ostringstream str_stream;
  str_stream << p2p << " start -ip " <<
                ip << " -key " << key <<
                " -hash " << hash << " &";
  std::string command = str_stream.str();
  return ssystem(command.c_str());
}
