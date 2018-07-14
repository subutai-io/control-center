#ifndef UPDATERCOMPONENTVAGRANTVBGUEST_H
#define UPDATERCOMPONENTVAGRANTVBGUEST_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVAGRANT_VBGUEST class implements
 * IUpdaterComponent. Works with vbguest plugin
 */
class CUpdaterComponentVAGRANT_VBGUEST : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentVAGRANT_VBGUEST();
  virtual ~CUpdaterComponentVAGRANT_VBGUEST();

  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual void install_post_interntal(bool success);
  virtual chue_t uninstall_internal();
  virtual void uninstall_post_internal(bool success);
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTVAGRANTVBGUEST_H
