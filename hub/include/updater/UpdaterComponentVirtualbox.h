#ifndef UPDATERCOMPONENTVIRTUALBOX_H
#define UPDATERCOMPONENTVIRTUALBOX_H
#include "updater/IUpdaterComponent.h"
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVIRTUALBOX class implements IUpdaterComponent. Works
 * with vagrant
 */
  class CUpdaterComponentVIRTUALBOX : public IUpdaterComponent {
    // IUpdaterComponent interface
  public:
    CUpdaterComponentVIRTUALBOX();
    virtual ~CUpdaterComponentVIRTUALBOX();
    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual void update_post_action(bool success);
    virtual chue_t install_internal();
    virtual void install_post_interntal(bool success);
    virtual chue_t uninstall_internal();
    virtual void uninstall_post_internal(bool success);
  private:
    QString download_virtualbox_path();
  };
}
#endif // UPDATERCOMPONENTVIRTUALBOX_H
