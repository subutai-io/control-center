#include "SystemCallWrapper.h"
#include"ApplicationLog.h"
#include "rtm/include/RtmRemoteController.h"

using namespace rtm;

proc_load_avg_t
CRtmRemoteController::parse_load_avg(const std::string &str) {
  proc_load_avg_t res;
  int sr = sscanf(str.c_str(),
              "%lf  %lf %lf %lu %*1[/] %lu %lu",
              &res.avg1, &res.avg5, &res.avg15,
              &res.current, &res.scheduled, &res.pid);

  if (sr != 6) {
    CApplicationLog::Instance()->LogError(
          "Couldn't parse average load. \"%s\". res : %d", str.c_str(), sr);
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////

proc_load_avg_t
CRtmRemoteController::load_average() {
  proc_load_avg_t res;
  static const char* cmd = "cat /proc/loadavg";
  std::vector<std::string> lst_out;
  int ec = -1;

  system_call_wrapper_error_t cr =
      CSystemCallWrapper::ssystem_th(cmd, lst_out, ec, true);

  if (cr != SCWE_SUCCESS) {
    //todo handle error
    return res;
  }

  if (lst_out.empty()) {
    //todo handle error2
    return res;
  }

  res = parse_load_avg(lst_out[0]);
  return res;
}
////////////////////////////////////////////////////////////////////////////

proc_meminfo_t
CRtmRemoteController::meminfo() {
  static const char* fs[] = {
    "MemTotal: %lu", "MemFree: %lu", "MemAvailable: %lu", "Buffers: %lu",
    "Cached: %lu", "SwapCahced: %lu", "Active: %lu", "Inactive: %lu",
    "Active(anon): %lu", "Inactive(anon): %lu", "Active(file): %lu", "Inactive(file): %lu",
    "Mlocked: %lu", "HighTotal: %lu", "HighFree: %lu", "LowTotal: %lu", "LowFree: %lu",
    "MmapCopy: %lu", "SwapTotal: %lu", "SwapFree: %lu", "Dirty: %lu", "WriteBack: %lu",
    "AnonPages: %lu", "Mapped: %lu", "Shmem: %lu", "Slab: %lu", "SReclaimable: %lu", "SUnreclaim: %lu",
    "KernelStack: %lu", "PageTables: %lu", "Quicklists: %lu", "NFS_Unstable: %lu", "Bounce: %lu",
    "WritebackTmp: %lu", "CommitLimit: %lu", "Commited_AS: %lu", "VmallocTotal: %lu",
    "VmallocUsed: %lu", "VmallocChunk: %lu", "HardwareCorrupted: %lu",
    "AnonHugePages: %lu", "CmaTotal: %lu", "CmaFree: %lu", "HugePages_Total: %lu",
    "HugePages_Free: %lu", "HugePages_Rsvd: %lu", "HugePages_Surp: %lu",
    "Hugepagesize: %lu", "DirectMap4k: %lu", "DirectMap4M: %lu",
    "DirectMap2M: %lu", "DirectMap1G: %lu"
  };
  static const char* cmd = "cat /proc/meminfo";

  struct field_meta_t {
    uint64_t* fp; //field pointer
    int fi;       //field index
  };

  proc_meminfo_t pm;
  memset(&pm, 0, sizeof(proc_meminfo_t));
  std::vector<std::string> lst_out;
  int ec = -1;

  system_call_wrapper_error_t cr =
      CSystemCallWrapper::ssystem_th(cmd, lst_out, ec, true);

  if (cr != SCWE_SUCCESS || lst_out.empty()) {
    //todo handle SytemCallWrapperError.
    return pm;
  }

  field_meta_t fm[] = {
    {&pm.mem_total, proc_meminfo_t::pmf_MemTotal},
    {&pm.mem_free, proc_meminfo_t::pmf_MemFree},
    {&pm.shmem, proc_meminfo_t::pmf_Shmem},
    {&pm.buffers, proc_meminfo_t::pmf_Buffers},
    {&pm.swap_total, proc_meminfo_t::pmf_SwapTotal},
    {&pm.swap_free, proc_meminfo_t::pmf_SwapFree},
    {&pm.high_total, proc_meminfo_t::pmf_HighTotal},
    {&pm.high_free, proc_meminfo_t::pmf_HighFree},
    {NULL, -1}
  };

  field_meta_t* ft = fm;
  for (; ft->fi != -1; ++ft) {
    if ((size_t)ft->fi >= lst_out.size()) continue;
    int pr = sscanf(lst_out[ft->fi].c_str(), fs[ft->fi], ft->fp);
    if (pr != 1) {
      //try to find in another lst_out results
      for (size_t i = 0; i < lst_out.size(); ++i) {
        pr = sscanf(lst_out[i].c_str(), fs[ft->fi], ft->fp);
        if (pr == 1) break;
      }
    }
  }

  return pm;
}
////////////////////////////////////////////////////////////////////////////
