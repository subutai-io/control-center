#include "InternalCriticalSection.h"

void SynchroPrimitives::EnterInternalCriticalSection(LPMCriticalSection lpMcs ) {
  lpMcs->mut.lock();
}
//////////////////////////////////////////////////////////////////////////

void SynchroPrimitives::LeaveInternalCriticalSection(LPMCriticalSection lpMcs ) {
  lpMcs->mut.unlock();
}
//////////////////////////////////////////////////////////////////////////

SynchroPrimitives::CriticalSection::CriticalSection() : mut(/*QMutex::Recursive*/) {
}
//////////////////////////////////////////////////////////////////////////
