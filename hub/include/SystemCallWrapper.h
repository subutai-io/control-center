#ifndef SYSTEMCALLWRAPPER_H
#define SYSTEMCALLWRAPPER_H

#include <string>
#include <vector>

class CSystemCallWrapper {
public:

  static std::vector<std::string> ssystem (const char *command);

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
  static std::vector<std::string> join_to_p2p_swarm (const char* hash,
                                                     const char* key,
                                                     const char* ip);


  static std::vector<std::string> p2p_swarms_presented();

  static void run_ssh_in_terminal(const char *user, const char *ip);
};

#endif // SYSTEMCALLWRAPPER_H

