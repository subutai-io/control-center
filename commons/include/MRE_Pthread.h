#ifndef MRE_LINUX_H
#define MRE_LINUX_H

#include <pthread.h>
#include "InternalCriticalSection.h"

namespace SynchroPrimitives {
  class CPthreadMRE {
  private:

    struct mre_info {
      pthread_mutex_t m_mutex;
      pthread_cond_t m_cond;
      bool m_signaled;
      CriticalSection m_cs;
    } m_mreInfo;

    CPthreadMRE(const CPthreadMRE&);
    void operator=(const CPthreadMRE&);

  public:
    CPthreadMRE(void){}
    ~CPthreadMRE(void){}

    static int MRE_Init(CPthreadMRE* lpMre);
    static int MRE_Wait(CPthreadMRE* lpMre, int timeInMs);
    static int MRE_Set(CPthreadMRE* lpMre);
    static int MRE_Reset(CPthreadMRE* lpMre);
    static int MRE_Destroy(CPthreadMRE* lpMre);
  };
}

#endif // MRE_LINUX_H
