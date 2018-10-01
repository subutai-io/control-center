#ifndef UPDATERCOMPONENTHYPERV_H
#define UPDATERCOMPONENTHYPERV_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentHyper  class implements IUpdaterComponent.
 * Works with vagrant
 */
class CUpdaterComponentHyperv : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentHyperv();
  virtual ~CUpdaterComponentHyperv();
  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual void install_post_internal(bool success);
  virtual chue_t uninstall_internal();
  virtual void uninstall_post_internal(bool success);
};
}
#endif // UPDATERCOMPONENTHYPERV_H
