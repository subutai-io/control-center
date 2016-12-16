#ifndef UPDATERCOMPONENTRH_H
#define UPDATERCOMPONENTRH_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /*!
   * \brief This class is used for managing resource host update system
   */
  class CUpdaterComponentRH : public IUpdaterComponent {
    // IUpdaterComponent interface
  public:
    CUpdaterComponentRH();
    virtual ~CUpdaterComponentRH();

    // IUpdaterComponent interface
  protected:
    /*!
     * \brief See IUpdaterComponent
     */
    virtual bool update_available_internal();

    /*!
     * \brief See IUpdaterComponent
     */
    virtual chue_t update_internal();

    /*!
     * \brief See IUpdaterComponent
     */
    virtual void update_post_action(bool success);
  };

}

#endif // UPDATERCOMPONENTRH_H
