#ifndef UPDATERCOMPONENTCHROME_H
#define UPDATERCOMPONENTCHROME_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
class CUpdaterComponentCHROME : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentCHROME();
  virtual ~CUpdaterComponentCHROME();

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
  QString download_chrome_path();
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTX2GO_H
