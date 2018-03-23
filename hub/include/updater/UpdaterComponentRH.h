#ifndef UPDATERCOMPONENTRH_H
#define UPDATERCOMPONENTRH_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /**
   * @brief The CUpdaterComponentRH class implements IUpdaterComponent. Works with RH (resource host)
   */
  class CUpdaterComponentRH : public IUpdaterComponent {
    // IUpdaterComponent interface
  public:
    CUpdaterComponentRH();
    virtual ~CUpdaterComponentRH();

    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action(bool success);
    virtual chue_t install_internal();
    virtual void install_post_interntal(bool success);
  };

}

#endif // UPDATERCOMPONENTRH_H
