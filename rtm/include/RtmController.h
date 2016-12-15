#ifndef RTMCONTROLLER_H
#define RTMCONTROLLER_H

#include <vector>
#include <string>
#include <stdint.h>
#include "RtmProcParser.h"

namespace rtm {

#pragma pack(push)
#pragma pack(1)

  struct sysinfo_t {
    int32_t  uptime;    /* Seconds since boot */
    uint32_t loads[3];  /* 1, 5, and 15 minute load averages */
    uint32_t totalram;  /* Total usable main memory size */
    uint32_t freeram;   /* Available memory size */
    uint32_t sharedram; /* Amount of shared memory */
    uint32_t bufferram; /* Memory used by buffers */
    uint32_t totalswap; /* Total swap space size */
    uint32_t freeswap;  /* swap space still available */
    uint16_t procs;     /* Number of current processes */
    uint32_t totalhigh; /* Total high memory size */
    uint32_t freehigh;  /* Available high memory size */
    uint32_t mem_unit;  /* Memory unit size in bytes */
//    char _f[20-2*sizeof(long)-sizeof(int)]; /* Padding to 64 bytes */
  };

#pragma pack(pop)

  class CRtmController {
  private:
//    static std::vector<std::string> get_output_by_syscall(const char* cmd, bool* success);
    static std::vector<std::string> get_output_by_libssh2(const char* cmd, bool* success);

//    CRtmProcParser* m_rpp_syscall;
    CRtmProcParser* m_rpp_libssh2;

    CRtmController();
    ~CRtmController();

  public:
    static CRtmController* Instance();

    sysinfo_t rtm_sysinfo() const;
    void Megatest();
  };

}

#endif // RTMCONTROLLER_H
