#ifndef MRE_LINUX_H
#define MRE_LINUX_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include "InternalCriticalSection.h"

namespace SynchroPrimitives {

  class CLinuxManualResetEvent {
  private:

    struct mre_info {
      std::mutex m_mutex;
      std::unique_lock<std::mutex> m_ul;
      std::condition_variable m_cond;
      bool m_signaled;
      CriticalSection m_cs;
    } m_mreInfo;

    CLinuxManualResetEvent(const CLinuxManualResetEvent&);
    void operator=(const CLinuxManualResetEvent&);

  public:
    CLinuxManualResetEvent(void){}
    ~CLinuxManualResetEvent(void){}

    static int MRE_Init(CLinuxManualResetEvent* lpMre);
    static int MRE_Wait(CLinuxManualResetEvent* lpMre, int timeInMs);
    static int MRE_Set(CLinuxManualResetEvent* lpMre);
    static int MRE_Reset(CLinuxManualResetEvent* lpMre);
    static int MRE_Destroy(CLinuxManualResetEvent* lpMre);
  };
}

#endif // MRE_LINUX_H
