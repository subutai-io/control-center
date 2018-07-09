#ifndef UPDATERCOMPONENTTRAY_H
#define UPDATERCOMPONENTTRAY_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /**
   * @brief The CUpdaterComponentTray class implements IUpdaterComponent. Works with SubutaiTray
   */
  class CUpdaterComponentTray : public IUpdaterComponent {
  private:
    static QString tray_path();
    static QString download_tray_path();
    QString tray_cloud_version;

    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action(bool success);
    virtual chue_t install_internal();
    virtual void install_post_interntal(bool success);

  public:
    CUpdaterComponentTray();
    virtual ~CUpdaterComponentTray();
  };
}

#endif // UPDATERCOMPONENTTRAY_H
