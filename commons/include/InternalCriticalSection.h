#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include <Commons.h>
#include <QMutex>

#define CRITICAL_SECTION_SLEEP_TIMEOUT 1000

namespace SynchroPrimitives
{
  /*!
    * \brief Critical section synchronization primitive.
    * Here can be used any OS mechanism, std::mutex or processors instructions like "BTS"
    * In SubutaiTray application std::mutex wrapped by CriticalSection structure.
    */
  typedef struct CriticalSection
  {
    QMutex mut;
    CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  /*!
   * \brief Enter critical section
   * \param - pointer to critical section object
   */
  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);

  /*!
   * \brief Leave critical section
   * \param - pointer to critical section object
   */
  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
