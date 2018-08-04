#ifndef UPDATERCOMPONENTXQUARTZ_H
#define UPDATERCOMPONENTXQUARTZ_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentXQuartz class implements IUpdaterComponent. Works
 * with XQuartz
 */
class CUpdaterComponentXQuartz : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentXQuartz();
  virtual ~CUpdaterComponentXQuartz();

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
  QString download_xquartz_path();
};
}  // namespace update_system
#endif // UPDATERCOMPONENTXQUARTZ_H
