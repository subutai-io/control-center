#ifndef UPDATERCOMPONENTRHMANAGEMENT_H
#define UPDATERCOMPONENTRHMANAGEMENT_H

#include "updater/IUpdaterComponent.h"

namespace update_system {
  class CUpdaterComponentRHM : public IUpdaterComponent {
  public:
    CUpdaterComponentRHM();
    virtual ~CUpdaterComponentRHM();

    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action(bool success);
  };
}

#endif // UPDATERCOMPONENTRHMANAGEMENT_H
