#ifndef UPDATERCOMPONENTP2P_H
#define UPDATERCOMPONENTP2P_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /**
   * @brief The CUpdaterComponentP2P class implements IUpdaterComponent. Works with p2p
   */
  class CUpdaterComponentP2P : public IUpdaterComponent {
  private:
    static QString p2p_path();
    static QString download_p2p_path();
    // IUpdaterComponent interface
  protected:
    virtual bool update_available_internal();
    virtual chue_t update_internal();
    virtual chue_t update_internal_mac_win();
    virtual chue_t update_internal_linux();
    virtual void update_post_action(bool success);
    virtual chue_t install_internal();
    virtual chue_t install_internal(bool update);
    virtual void install_post_internal(bool success);
    virtual chue_t uninstall_internal();
    virtual void uninstall_post_internal(bool success);

  public:
    CUpdaterComponentP2P();
    virtual ~CUpdaterComponentP2P();
  };
  class PostUpdater : public QObject{
      Q_OBJECT
  public:
      PostUpdater(QObject *parent = nullptr) : QObject (parent){}
      void start_work();
      void post_update();
  signals:
      void output_received(bool success);
  };
}
#endif // UPDATERCOMPONENTP2P_H
