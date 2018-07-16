#ifndef UPDATERCOMPONENTSUBUTAIBOX_H
#define UPDATERCOMPONENTSUBUTAIBOX_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentSUBUTAI_BOX class implements IUpdaterComponent.
 * Works with vagrant subutai box
 */
class CUpdaterComponentSUBUTAI_BOX : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentSUBUTAI_BOX();
  virtual ~CUpdaterComponentSUBUTAI_BOX();

  // IUpdaterComponent interface
 protected:
  virtual bool update_available_internal();
  virtual chue_t update_internal();
  virtual void update_post_action(bool success);
  virtual chue_t install_internal();
  virtual chue_t install_internal(bool update);
  virtual void install_post_internal(bool success);
  virtual chue_t uninstall_internal();
  virtual void uninstall_post_internal(bool success);

 private:
  QString download_subutai_box_path();
};
}  // namespace update_system
#endif  // UPDATERCOMPONENTSUBUTAIBOX_H
