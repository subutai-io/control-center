#include <stdio.h>
#include <errno.h>

#include "MRE_Linux.h"
#include "Locker.h"

int SynchroPrimitives::CLinuxManualResetEvent::MRE_Wait(CLinuxManualResetEvent *lpMre, int timeInMs) {
  std::cv_status res;
  lpMre->m_mreInfo.m_mutex.lock();

  while(!lpMre->m_mreInfo.m_signaled) {
    res = lpMre->m_mreInfo.m_cond.wait_for(lpMre->m_mreInfo.m_ul,
                                           std::chrono::milliseconds(timeInMs));
    if (res == std::cv_status::timeout) break;
  }
  lpMre->m_mreInfo.m_mutex.unlock();
  SynchroPrimitives::Locker lock(&lpMre->m_mreInfo.m_cs);
  lpMre->m_mreInfo.m_signaled = true;
  return 0;
}
//////////////////////////////////////////////////////////////////////////

int SynchroPrimitives::CLinuxManualResetEvent::MRE_Set(CLinuxManualResetEvent *lpMre) {
  SynchroPrimitives::Locker lock(&lpMre->m_mreInfo.m_cs);
  lpMre->m_mreInfo.m_signaled = true;
  lpMre->m_mreInfo.m_cond.notify_one();
  lpMre->m_mreInfo.m_mutex.unlock();
  return 0;
}
//////////////////////////////////////////////////////////////////////////

int SynchroPrimitives::CLinuxManualResetEvent::MRE_Reset(CLinuxManualResetEvent *lpMre) {
  SynchroPrimitives::Locker lock(&lpMre->m_mreInfo.m_cs);
  lpMre->m_mreInfo.m_signaled = false;
  return 0;
}
//////////////////////////////////////////////////////////////////////////

int SynchroPrimitives::CLinuxManualResetEvent::MRE_Init(CLinuxManualResetEvent *lpMre) {
  lpMre->m_mreInfo.m_signaled = false;
  lpMre->m_mreInfo.m_ul = std::unique_lock<std::mutex>(lpMre->m_mreInfo.m_mutex);
  return 0;
}
//////////////////////////////////////////////////////////////////////////

int SynchroPrimitives::CLinuxManualResetEvent::MRE_Destroy(CLinuxManualResetEvent *lpMre) {
  (void)lpMre;
  return 0;
}
//////////////////////////////////////////////////////////////////////////
