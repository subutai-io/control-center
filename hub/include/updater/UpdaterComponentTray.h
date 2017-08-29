#ifndef UPDATERCOMPONENTTRAY_H
#define UPDATERCOMPONENTTRAY_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  class CUpdaterComponentTray : public IUpdaterComponent {
  private:
    static QString tray_path();
    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action(bool success);

  public:
    CUpdaterComponentTray();
    virtual ~CUpdaterComponentTray();
  };
}

#endif // UPDATERCOMPONENTTRAY_H
