#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <string>
#include <vector>
#include <QString>

enum system_call_wrapper_error_t {
  /*common errors*/
  SCWE_SUCCESS = 0,
  SCWE_SHELL_ERROR,
  SCWE_PIPE,
  SCWE_SET_HANDLE_INFO,
  SCWE_CREATE_PROCESS,

  /*p2p errors*/
  SCWE_CANT_JOIN_SWARM,

  /*ssh errors*/
  SCWE_SSH_LAUNCH_FAILED,
};
////////////////////////////////////////////////////////////////////////////

class CSystemCallWrapper {
private:
  static system_call_wrapper_error_t ssystem(const char *command, std::vector<std::string> &lst_output);

public:
  /*
    -dev interface name
        TUN/TAP interface name
    -dht HOST:PORT
        Specify DHT bootstrap node address in a form of HOST:PORT
    -hash Infohash
        Infohash for environment
    -ip IP
        IP address to be used (default "none")
    -key string
        AES crypto key
    -keyfile string
        Path to yaml file containing crypto key
    -mac Hardware Address
        MAC or Hardware Address for a TUN/TAP interface
    -mask subnet
        Network mask a.k.a. subnet (default "255.255.255.0")
    -port Port
        Port that will be used for p2p communication. Random port number will be generated if no port were specified
    -ttl string
        Time until specified key will be available

  */

  static bool is_in_swarm(const char* hash);

  static system_call_wrapper_error_t join_to_p2p_swarm(const char* hash,
                                                       const char* key,
                                                       const char* ip);

  static system_call_wrapper_error_t run_ssh_in_terminal(const char *user,
                                                         const char *ip);
  static system_call_wrapper_error_t fork_process(const QString program,
                                                  const QStringList argv);
  static system_call_wrapper_error_t open_url(QString s_url);
};

#endif // SYSTEMCALLWRAPPER_H

