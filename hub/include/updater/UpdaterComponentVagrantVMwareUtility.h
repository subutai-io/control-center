#ifndef UPDATERCOMPONENTVAGRANTVMWAREUTILITY_H
#define UPDATERCOMPONENTVAGRANTVMWAREUTILITY_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVagrantVMwareUtility class implements
 * IUpdaterComponent.
 */
class CUpdaterComponentVagrantVMwareUtility : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentVagrantVMwareUtility();
  virtual ~CUpdaterComponentVagrantVMwareUtility();

  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual chue_t uninstall_internal();
  virtual void install_post_interntal(bool success);
  virtual void uninstall_post_internal(bool success);

private:
 QString download_vmware_utility_path();
};
}
#endif // UPDATERCOMPONENTVAGRANTVMWAREUTILITY_H
