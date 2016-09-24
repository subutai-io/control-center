#include "SystemCallWrapper.h"
#include"ApplicationLog.h"
#include "rtm/include/RtmProcParser.h"

using namespace rtm;

proc_meminfo_t
CRtmProcParser::create_meminfo() {
  proc_meminfo_t pm;
  memset(&pm, 0, sizeof(proc_meminfo_t));
  return pm;
}

proc_net_dev_t
CRtmProcParser::create_net_dev() {
  proc_net_dev_t nd;
  memset(&nd, 0, sizeof(proc_net_dev_t));
  return nd;
}
////////////////////////////////////////////////////////////////////////////

proc_load_avg_t
CRtmProcParser::load_average() const {
  static const char* cmd = "cat /proc/loadavg";
  static const char* fmt = "%lf  %lf %lf %lu %*1[/] %lu %lu";
  proc_load_avg_t res;
  bool success = false;
  std::vector<std::string> lst_out = m_read_f(cmd, &success);

  int sr = sscanf(lst_out[0].c_str(), fmt,
              &res.avg1, &res.avg5, &res.avg15,
              &res.current, &res.scheduled, &res.pid);

  if (sr != 6) {
    CApplicationLog::Instance()->LogError(
          "Couldn't parse average load. \"%s\". res : %d", lst_out[0].c_str(), sr);
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////

struct meminfo_field_meta_t {
  uint64_t* fp; //field pointer
  int fi;       //field index
};

proc_meminfo_t
CRtmProcParser::meminfo() const {
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
  proc_meminfo_t pm = create_meminfo();
  bool success = false;
  std::vector<std::string> lst_out = m_read_f(cmd, &success);

  if (!success || lst_out.empty()) {
    CApplicationLog::Instance()->LogError("Couldn't get meminfo output");
    return pm;
  }

  meminfo_field_meta_t fm[] = {
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

  meminfo_field_meta_t* ft = fm;
  for (; ft->fi != -1; ++ft) {
    if ((size_t)ft->fi >= lst_out.size()) continue;
    int pr = sscanf(lst_out[ft->fi].c_str(), fs[ft->fi], ft->fp);
    if (pr != 1) {
      //try to find in another lst_out results
      for (size_t i = 0; i < lst_out.size() && pr != 1; ++i) {
        pr = sscanf(lst_out[i].c_str(), fs[ft->fi], ft->fp);
      }
    }
  }

  return pm;
}
////////////////////////////////////////////////////////////////////////////

proc_uptime_t
CRtmProcParser::uptime() const {
  static const char* cmd = "cat /proc/uptime";
  static const char* fmt = "%lf %lf";
  proc_uptime_t res;
  bool success = false;
  std::vector<std::string> lst_out = m_read_f(cmd, &success);

  if (!success || lst_out.empty()) {
    CApplicationLog::Instance()->LogError("cat /proc/uptime call failed");
    return res;
  }

  int pr = sscanf(lst_out[0].c_str(), fmt, &res.uptime, &res.idle);

  if (pr != 2) {
    CApplicationLog::Instance()->LogError(
          "sscanf failed. str : %s, pr = %d", lst_out[0].c_str());
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////

std::vector<proc_net_dev_t>
CRtmProcParser::network_info() const {
  static const char* cmd = "cat /proc/net/dev";
  static const char* fmt = "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu";
  std::vector<proc_net_dev_t> lst_res;
  bool success = false;
  std::vector<std::string> lst_out = m_read_f(cmd, &success);

  if (!success || lst_out.empty()) {
    CApplicationLog::Instance()->LogError("cat /proc/net/dev call failed");
    return lst_res;
  }

  proc_net_dev_t nd = create_net_dev();
  for (size_t i = 2; i < lst_out.size(); ++i) {
    int pr = sscanf(lst_out[i].c_str(), fmt, nd.if_name,
                    &nd.recv.bytes, &nd.recv.packets, &nd.recv.errs, &nd.recv.drop,
                    &nd.recv.fifo, &nd.recv.frame, &nd.recv.compressed, &nd.recv.multicast,
                    &nd.trans.bytes, &nd.trans.packets, &nd.trans.errs, &nd.trans.drop,
                    &nd.trans.fifo, &nd.trans.colls, &nd.trans.carrier, &nd.trans.compressed);
    if (pr != 17) {
      CApplicationLog::Instance()->LogError("Couldn't parse \"%s\". pr = %d", lst_out[i].c_str(), pr);
      continue;
    }
    lst_res.push_back(nd);
  }

  return lst_res;
}
////////////////////////////////////////////////////////////////////////////
