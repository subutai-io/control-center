#ifndef MRE_WINDOWS_H
#define MRE_WINDOWS_H

#include <QUuid>
#include "InternalCriticalSection.h"

namespace SynchroPrimitives {

  /*!
   * \brief Windows manual reset event wrapper (using OS specific functions)
   */
  class CWindowsManualResetEvent {
  private:
    HANDLE m_mre;

    CWindowsManualResetEvent(const CWindowsManualResetEvent&);
    void operator=(const CWindowsManualResetEvent&);

  public:
    CWindowsManualResetEvent(void){}
    ~CWindowsManualResetEvent(void){}

    static int MRE_Init(CWindowsManualResetEvent* lpMre) {
      lpMre->m_mre = CreateEventA(NULL, TRUE, FALSE, QUuid::createUuid().toString().toStdString().c_str()) ;
      return 0;
    }
    static int MRE_Wait(CWindowsManualResetEvent* lpMre, int timeInMs) { return WaitForSingleObject (lpMre->m_mre, timeInMs) ;}
    static int MRE_Set(CWindowsManualResetEvent* lpMre) {return SetEvent(lpMre->m_mre);}
    static int MRE_Reset(CWindowsManualResetEvent* lpMre) {return ResetEvent(lpMre->m_mre);}
    static int MRE_Destroy(CWindowsManualResetEvent* lpMre) {return CloseHandle(lpMre->m_mre);}
  };
}

#endif // MRE_WINDOWS_H
