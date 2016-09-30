#ifndef RTMCONTROLLER_H
#define RTMCONTROLLER_H

#include <vector>
#include <string>
#include "RtmProcParser.h"

namespace rtm {

  class CRtmController {
  private:
    static std::vector<std::string> get_output_by_syscall(const char* cmd, bool* success);
    static std::vector<std::string> get_output_by_libssh2(const char* cmd, bool* success);

    CRtmProcParser* m_rpp_syscall;
    CRtmProcParser* m_rpp_libssh2;

    CRtmController();
    ~CRtmController();

  public:
    static CRtmController* Instance();

    void Megatest();
  };

}

#endif // RTMCONTROLLER_H
