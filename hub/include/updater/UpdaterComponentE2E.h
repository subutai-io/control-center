#ifndef UPDATERCOMPONENTE2E_H
#define UPDATERCOMPONENTE2E_H

#endif  // UPDATERCOMPONENTE2E_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
/**
 * @brief The CUpdaterComponentE2E class implements IUpdaterComponent. Works
 * with subutai e2e plugin
 */
class CUpdaterComponentE2E : public IUpdaterComponent {
  // IUpdaterComponent interface
 public:
  CUpdaterComponentE2E();
  virtual ~CUpdaterComponentE2E();

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
  QString download_e2e_path();
};
}  // namespace update_system
