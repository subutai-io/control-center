#ifndef UPDATERCOMPONENTTRAY_H
#define UPDATERCOMPONENTTRAY_H

#include "updater/IUpdaterComponent.h"

namespace update_system {

  /*!
   * \brief Class for managing tray application updates
   */
  class CUpdaterComponentTray : public IUpdaterComponent {

  private:

    static QString tray_path();

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
    virtual void update_post_action(bool success);

  public:
    CUpdaterComponentTray();
    virtual ~CUpdaterComponentTray();

    /*!
     * \brief OS specific names of tray executable on kurjun
     */
    static const char *tray_kurjun_file_name();
  };
}

#endif // UPDATERCOMPONENTTRAY_H
