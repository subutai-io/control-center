#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include <Commons.h>
#include <QMutex>

#define CRITICAL_SECTION_SLEEP_TIMEOUT 1000

namespace SynchroPrimitives
{
  /**
    Wrapper for critical section abstraction.
    It could be windows critical section, mutex or anything else.
    For example it's possible to use asm("bts") instruction.
  */
  typedef struct CriticalSection
  {
    /*we use QMutex, but it could be anything else.
      for examle we can use asm("bts") instruction*/
    QMutex mut;
    CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  /**
   * @brief Enter critical section
   * @param lpMcs - pointer to CriticalSection
   */
  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);

  /**
   * @brief Leave critical section
   * @param lpMcs - pointer to CriticalSection
   */
  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
