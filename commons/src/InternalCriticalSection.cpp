#include <unistd.h>
#include "InternalCriticalSection.h"

void SynchroPrimitives::EnterInternalCriticalSection(LPMCriticalSection lpMcs ) {
  lpMcs->mut.lock();
}
//////////////////////////////////////////////////////////////////////////

void SynchroPrimitives::LeaveInternalCriticalSection(LPMCriticalSection lpMcs ) {
  lpMcs->mut.unlock();
}
//////////////////////////////////////////////////////////////////////////

/*init critical section :)*/
SynchroPrimitives::CriticalSection::CriticalSection() {  
}
//////////////////////////////////////////////////////////////////////////
