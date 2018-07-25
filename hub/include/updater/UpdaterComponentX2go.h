#ifndef UPDATERCOMPONENTX2GO_H
#define UPDATERCOMPONENTX2GO_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentX2GO class implements IUpdaterComponent. Works
 * with X2GO
 */
class CUpdaterComponentX2GO : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentX2GO();
  virtual ~CUpdaterComponentX2GO();

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
  QString download_x2go_path();
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTX2GO_H
