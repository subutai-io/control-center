#ifndef UPDATERCOMPONENTP2P_H
#define UPDATERCOMPONENTP2P_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /*!
   * \brief This class is used for managing p2p update system
   */
  class CUpdaterComponentP2P : public IUpdaterComponent {
  private:
    /*!
     * \brief Path of p2p executable file. Could be value in Settings or result of `which` (`where` on Windows) command
     */
    static std::string p2p_path();

    // IUpdaterComponent interface
  protected:
    /*!
     * \brief See IUpdaterComponent
     */
    virtual bool update_available_internal();

    /*!
     * \brief See IUpdaterComponent
     */
    virtual chue_t update_internal();

    /*!
     * \brief See IUpdaterComponent
     */
    virtual void update_post_action();

  public:
    CUpdaterComponentP2P();
    virtual ~CUpdaterComponentP2P();
  };
}

#endif // UPDATERCOMPONENTP2P_H
