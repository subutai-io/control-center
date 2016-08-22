#ifndef UPDATERCOMPONENTRH_H
#define UPDATERCOMPONENTRH_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  class CUpdaterComponentRH : public IUpdaterComponent {

    // IUpdaterComponent interface
  public:
    CUpdaterComponentRH();
    virtual ~CUpdaterComponentRH();

    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action();
  };

}

#endif // UPDATERCOMPONENTRH_H
