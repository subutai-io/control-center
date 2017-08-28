#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include <Commons.h>
#include <QMutex>

#define CRITICAL_SECTION_SLEEP_TIMEOUT 1000

namespace SynchroPrimitives
{
  typedef struct CriticalSection
  {
    /*we use QMutex, but it could be anything else.
      for examle we can use asm("bts") instruction*/
    QMutex mut;
    CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);

  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
