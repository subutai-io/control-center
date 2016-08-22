#ifndef UPDATERCOMPONENTTRAY_H
#define UPDATERCOMPONENTTRAY_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  class CUpdaterComponentTray : public IUpdaterComponent {


  private:
    static std::string tray_path();

    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action();

  public:
    CUpdaterComponentTray();
    virtual ~CUpdaterComponentTray();
    static const char *tray_kurjun_file_name();
  };
}

#endif // UPDATERCOMPONENTTRAY_H
