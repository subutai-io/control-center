#ifndef UPDATERCOMPONENTVAGRANTSUBUTAI_H
#define UPDATERCOMPONENTVAGRANTSUBUTAI_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentVAGRANT_SUBUTAI class implements
 * IUpdaterComponent. Works with subutai subutai plugin
 */
class CUpdaterComponentVAGRANT_SUBUTAI : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentVAGRANT_SUBUTAI();
  virtual ~CUpdaterComponentVAGRANT_SUBUTAI();

  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual chue_t uninstall_internal();
  virtual void install_post_interntal(bool success);
  virtual void uninstall_post_internal(bool success);
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTVAGRANTSUBUTAI_H
