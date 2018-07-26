#ifndef UPDATERCOMPONENTVMWAREWORKSTATION_H
#define UPDATERCOMPONENTVMWAREWORKSTATION_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVIRTUALBOX class implements IUpdaterComponent.
 * Works with vagrant
 */
class CUpdaterComponentVMware : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentVMware();
  virtual ~CUpdaterComponentVMware();
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
  QString download_vmware_path();
};
}
#endif // UPDATERCOMPONENTVMWAREWORKSTATION_H
