#ifndef LOCK_H
#define LOCK_H

#include "InternalCriticalSection.h"

namespace SynchroPrimitives {

  /**
   * @brief The Locker guaranteed releases it's critical section
   * Should be used as stack object, not heap!!!
   * Example :
   * static SynchroPrimitives::CriticalSection cs;
   * void foo() {
   *   int a, b, c;
   *   b = c = ++a + a++;
   *   {
   *     Locker lock(&cs);
   *     //do some actions
   *   } //leave critical section (because of destruction of lock object)
   * }
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
