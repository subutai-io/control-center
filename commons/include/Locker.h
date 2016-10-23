#ifndef LOCK_H
#define LOCK_H

#include "InternalCriticalSection.h"

namespace SynchroPrimitives {

  /*!
   * \brief The Locker class provides critical sections management mechanism.
   * Use like this :
   * {Locker lock; / * some critical code here * /;}
   */
  class Locker {
  private:
    Locker(const Locker&);
    void operator=(const Locker&);
    Locker(void);

    CriticalSection* m_lpCs;
  public:
    Locker(CriticalSection *lpCs) : m_lpCs(lpCs) {SynchroPrimitives::EnterInternalCriticalSection(m_lpCs);}
    ~Locker(void) {SynchroPrimitives::LeaveInternalCriticalSection(m_lpCs);}
  };
}
#endif // LOCK_H
