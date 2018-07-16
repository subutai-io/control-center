#ifndef UPDATERCOMPONENTFIREFOX_H
#define UPDATERCOMPONENTFIREFOX_H
#include "updater/IUpdaterComponent.h"

namespace update_system {
class CUpdaterComponentFIREFOX : public IUpdaterComponent {
public:
  CUpdaterComponentFIREFOX();
  virtual ~CUpdaterComponentFIREFOX();

protected:
 virtual bool update_available_internal();
 virtual chue_t update_internal();
 virtual void update_post_action(bool success);
 virtual chue_t install_internal();
 virtual void install_post_internal(bool success);
 virtual chue_t uninstall_internal();
 virtual void uninstall_post_internal(bool success);

private:
 QString download_firefox_path();
};
}

#endif // UPDATERCOMPONENTFIREFOX_H
