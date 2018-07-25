#ifndef UPDATERCOMPONENTVAGRANT_H
#define UPDATERCOMPONENTVAGRANT_H

#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVAGRANT class implements IUpdaterComponent. Works
 * with vagrant
 */
class CUpdaterComponentVAGRANT : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentVAGRANT();
  virtual ~CUpdaterComponentVAGRANT();

  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual void install_post_internal(bool success);
  virtual chue_t uninstall_internal();
  virtual void uninstall_post_internal(bool success);

 private:
  QString download_vagrant_path();
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTVAGRANT_H
