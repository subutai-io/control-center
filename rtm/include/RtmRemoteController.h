#ifndef RTMCONTROLLER_H
#define RTMCONTROLLER_H

#include <stdint.h>
#include <string>

namespace rtm {

#pragma pack(push)
#pragma pack(1)


  struct proc_load_avg_t {
    enum {la_avg1 = 0, la_avg5, la_avg15,
          la_curr_sched, la_pid, la_last};

    double avg1, avg5, avg15;
    uint64_t current, scheduled, pid; //i guess it's unsigned long.

    proc_load_avg_t() :
      avg1(0.0), avg5(0.0), avg15(0.0),
      current(0), scheduled(0), pid(0) {}
  };
  ////////////////////////////////////////////////////////////////////////////


/*
  struct sysinfo {
    int64_t uptime;             // Seconds since boot
    uint64_t loads[3];  // 1, 5, and 15 minute load averages
    uint64_t totalram;  // Total usable main memory size
    uint64_t freeram;   // Available memory size
    uint64_t sharedram; // Amount of shared memory
    uint64_t bufferram; // Memory used by buffers
    uint64_t totalswap; // Total swap space size
    uint64_t freeswap;  // Swap space still available
    uint16_t procs;    // Number of current processes
    uint64_t totalhigh; // Total high memory size
    uint64_t freehigh;  // Available high memory size
    uint32_t mem_unit;   // Memory unit size in bytes
    //char _f; // padding to something 16 degree.
    //char _f[20-2*sizeof(long)-sizeof(int)]; // Padding to 64 bytes
  };
*/
  //todo use only necessary fields because size of this struct is 418 (as I remember)
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

    uint64_t mem_total, mem_free, /*mem_available, */ buffers;
//    uint64_t cached, swap_cahced, active, inactive;
//    uint64_t active_anon, inactive_anon, active_file, inactive_file;
    uint64_t /*mlocked, */high_total, high_free/*, low_total, low_free*/;
    uint64_t /*mmap_copy,*/ swap_total, swap_free/*, dirty, write_back*/;
    uint64_t /*anon_pages, mapped, */shmem/*, slab, sreclaimable, sunreclaim*/;
//    uint64_t kernel_stack, page_tables, quicklists, nfs_unstable, bounce;
//    uint64_t write_back_tmp, commit_limit, commited_as, vmalloc_total;
//    uint64_t vmalloc_used, vmalloc_chunk, hardware_corrupted;
//    uint64_t anon_huge_pages, cma_total, cma_free, huge_pages_total;
//    uint64_t huge_pages_free, huge_pages_reserved, huge_pages_surp;
//    uint64_t huge_page_size, direct_map_4K, direct_map_4M, direct_map_2M, direct_map_1G;
  };

#pragma pack(pop)

  enum rtm_result_t {
    RTM_SUCCESS = 0,
    RTM_FAILED
  };

  class CRtmRemoteController {
  private:
    static proc_load_avg_t parse_load_avg(const std::string& str);

  public:

    static proc_load_avg_t load_average();
    static proc_meminfo_t  meminfo();
  };

}

#endif // RTMCONTROLLER_H
