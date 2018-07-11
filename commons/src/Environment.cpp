#include "Environment.h"
#include "OsBranchConsts.h"
#include <thread>

#ifdef RT_OS_LINUX
#include <sys/sysinfo.h>
#endif

Environment* Environment::Instance() {
  static Environment env;
  return &env;
}

template <>
unsigned int Environment::ramSizeInternal <Os2Type <OS_LINUX> > () {
  struct sysinfo info;

  if (sysinfo(&info) == 0) {
    qDebug() << "RAM: "
             << " total: "
             << (int) (info.totalram * info.mem_unit / (1024 * 1024))
             << " free: "
             << (int) (info.freeram * info.mem_unit / (1024 * 1024));

    return (int) (info.totalram * info.mem_unit / (1024 * 1024));
  }

  return 0; // if can't get ram info
}

template <>
unsigned int Environment::ramSizeInternal <Os2Type <OS_MAC> > () {
  unsigned int n_cpu = std::thread::hardware_concurrency();
  qDebug() << "Darwin CPU: "
           << n_cpu;
  return n_cpu;
}

template <>
unsigned int Environment::ramSizeInternal <Os2Type <OS_WIN> > () {
  unsigned int n_cpu = std::thread::hardware_concurrency();
  qDebug() << "Windows CPU: "
           << n_cpu;
  return n_cpu;
}

unsigned int Environment::ramSize() {
  return ramSizeInternal <Os2Type <CURRENT_OS>>();
}

unsigned int Environment::numCpu() {
  unsigned int n_cpu = std::thread::hardware_concurrency();
  qDebug() << "CPU: "
           << n_cpu;

  return n_cpu;
}
