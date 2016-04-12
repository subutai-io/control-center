#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include <Commons.h>
#include <mutex>

#define CRITICAL_SECTION_SLEEP_TIMEOUT 1000

namespace SynchroPrimitives
{
  typedef struct CriticalSection
  {
    std::mutex mut;
    CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);
  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
