#ifndef RTMPROCPARSER_H
#define RTMPROCPARSER_H

#include <stdint.h>
#include <string>
#include <vector>

namespace rtm {

#pragma pack(push)
#pragma pack(1)

  struct proc_load_avg_t {
    enum {la_avg1 = 0, la_avg5, la_avg15,
          la_curr_sched, la_pid, la_last};

    uint32_t avg1, avg5, avg15;
    uint64_t current, scheduled, pid; //i guess it's unsigned long.

    proc_load_avg_t() :
      avg1(0.0f), avg5(0.0f), avg15(0.0f),
      current(0), scheduled(0), pid(0) {}
  };
  ////////////////////////////////////////////////////////////////////////////

  struct proc_meminfo_t {
    enum proc_meminfo_fields {
      pmf_MemTotal, pmf_MemFree, pmf_MemAvailable, pmf_Buffers,
      pmf_Cached, pmf_SwapCahced, pmf_Active, pmf_Inactive,
      pmf_Active_anon, pmf_Inactive_anon, pmf_Active_file, pmf_Inactive_file,
      pmf_Mlocked, pmf_HighTotal, pmf_HighFree, pmf_LowTotal, pmf_LowFree,
      pmf_MmapCopy, pmf_SwapTotal, pmf_SwapFree, pmf_Dirty, pmf_WriteBack,
      pmf_AnonPages, pmf_Mapped, pmf_Shmem, pmf_Slab, pmf_SReclaimable, pmf_SUnreclaim,
      pmf_KernelStack, pmf_PageTables, pmf_Quicklists, pmf_NFS_Unstable, pmf_Bounce,
      pmf_WritebackTmp, pmf_CommitLimit, pmf_Commited_AS, pmf_VmallocTotal,
      pmf_VmallocUsed, pmf_VmallocChunk, pmf_HardwareCorrupted,
      pmf_AnonHugePages, pmf_CmaTotal, pmf_CmaFree, pmf_HugePages_Total,
      pmf_HugePages_Free, pmf_HugePages_Rsvd, pmf_HugePages_Surp,
      pmf_Hugepagesize, pmf_DirectMap4k, pmf_DirectMap4M,
      pmf_DirectMap2M, pmf_DirectMap1G
    };

    uint64_t mem_total, mem_free, buffers, high_total, high_free;
    uint64_t swap_total, swap_free, shmem;
  };
  ////////////////////////////////////////////////////////////////////////////

  struct proc_uptime_t {
    double uptime, idle;
    proc_uptime_t() : uptime(0.0), idle(0.0){}
  };
  ////////////////////////////////////////////////////////////////////////////

  static const int NET_IF_NAMESIZE = 16;
  struct proc_net_dev_t {
    uint8_t if_name[NET_IF_NAMESIZE];
    struct recv_t {
      uint64_t bytes, packets, errs, drop;
      uint64_t fifo, frame, compressed, multicast;
    } recv;
    struct trans_t {
      uint64_t bytes, packets, errs, drop;
      uint64_t fifo, colls, carrier, compressed;
    } trans;
  };
  ////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

  typedef std::vector<std::string> (*pf_output_read)(const char*/*cmd*/, bool*/*result success*/);

  class CRtmProcParser {
  private:
    static proc_meminfo_t create_meminfo();
    static proc_net_dev_t create_net_dev();

    pf_output_read m_read_f;
  public:

    explicit CRtmProcParser(pf_output_read pf_r) : m_read_f(pf_r){}
    ~CRtmProcParser(){}

    proc_load_avg_t load_average(bool &success) const;
    proc_meminfo_t meminfo(bool &success) const;
    proc_uptime_t uptime(bool &success) const;
    std::vector<proc_net_dev_t> network_info(bool &success) const;
  };

}

#endif // RTMCONTROLLER_H
