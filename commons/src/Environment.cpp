#include "Environment.h"
#include "OsBranchConsts.h"
#include <thread>

Environment* Environment::Instance() {
  static Environment env;
  return &env;
}

template <>
unsigned int Environment::ramSizeInternal <Os2Type <OS_LINUX> > () {
  unsigned int n_cpu = std::thread::hardware_concurrency();
  qDebug() << "Linux CPU: "
           << n_cpu;
  return n_cpu;
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
