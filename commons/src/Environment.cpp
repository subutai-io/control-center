#include "Environment.h"
#include "OsBranchConsts.h"
#include "SystemCallWrapper.h"
#include <QStorageInfo>
#include <thread>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#ifdef RT_OS_LINUX
#include <sys/sysinfo.h>
#endif

#ifdef RT_OS_WINDOWS
#define _WIN32_WINNT  0x0501
#include <Windows.h>
#include <cstdlib>
#endif

#ifdef RT_OS_DARWIN
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

Environment* Environment::Instance() {
  static Environment env;
  return &env;
}

#ifdef RT_OS_LINUX
unsigned int Environment::ramSize() {
  struct sysinfo info;

  if (sysinfo(&info) == 0) {
    qDebug() << "RAM: "
             << " total: "
             << (int) (info.totalram * info.mem_unit / (1024 * 1024))
             << " free: "
             << (int) (info.freeram * info.mem_unit / (1024 * 1024));

    return (int) (info.totalram * info.mem_unit / (1024 * 1024)); //in MB
  }

  return 0; // if can't get ram info
}
#endif

#ifdef RT_OS_WINDOWS
unsigned int Environment::ramSize() {
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof (statex);
  GlobalMemoryStatusEx (&statex);

  qDebug() << "RAM: "
           << " total: "
           << (float)statex.ullTotalPhys/(1024*1024)
           << " free: "
           << (float)statex.ullAvailPhys/(1024*1024);

  return (float)statex.ullTotalPhys/(1024*1024); // in MB
}
#endif

#ifdef RT_OS_DARWIN
unsigned int Environment::ramSize() {
  int mib[2];
  int64_t physical_memory;
  size_t length;

  // Get the Physical memory size
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;
  length = sizeof(int64_t);
  sysctl(mib, 2, &physical_memory, &length, nullptr, 0);
  qDebug() << "RAM: "
           << (int)(physical_memory/(1024*1024));

  return (int)(physical_memory/(1024*1024)); //in MB
}
#endif

unsigned int Environment::numCpu() {
  unsigned int n_cpu = std::thread::hardware_concurrency();
  qDebug() << "CPU: "
           << n_cpu;

  return n_cpu;
}

// Will be implemented get disk size by Hypervisor disk storage path.
unsigned int Environment::diskSize() {
  // On Virtualbox MAXIMUM Virtual Disk size is 2 TB.
  QStorageInfo storage;
  QString path = VagrantProvider::Instance()->VmStorage();
  storage.setPath(path);
  qDebug() << storage.rootPath();
  if (storage.isReadOnly())
      qDebug() << "isReadOnly:" << storage.isReadOnly();

  qDebug() << "Check disk path: " << path;
  qDebug() << "Provider: " << VagrantProvider::Instance()->CurrentVal();
  qDebug() << "name:" << storage.name();
  qDebug() << "fileSystemType:" << storage.fileSystemType();
  qDebug() << "size:" << storage.bytesTotal()/1024/1024/1024 << "GB";
  qDebug() << "availableSize:" << storage.bytesAvailable()/1024/1024/1024 << "GB";
  return storage.bytesTotal()/1024/1024/1024;
}

// only for linux
bool Environment::isCpuSupport() {
  // egrep -c '(vmx|svm)' /proc/cpuinfo
  // output "0" it means that your CPU doesn't support hardware virtualization.
  // ouput "1" or more it does - but you still need to make sure that virtualization is enabled in the BIOS.
  QString cmd("cat");
  QStringList args;
  args << "/proc/cpuinfo";
  CSystemCallWrapper::which("cmd", cmd);

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 5000);

  if (res.exit_code == 0 && !res.out.empty() && res.res == SCWE_SUCCESS) {
    for (QString str : res.out) {
      if (str.contains("vmx") || str.contains("svm")) {
        return true;
      }
    }
  }

  return false;
}
