#include <map>

#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>

#include "updater/IUpdaterComponent.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgNotification.h"
#include "SettingsManager.h"
#include "HubController.h"
#include "Commons.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "DownloadFileManager.h"
#include "TrayControlWindow.h"

using namespace update_system;

#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::P2P = "P2P";
#else
const QString IUpdaterComponent::P2P = "p2p.exe";
#endif
#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter";
#else
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter.exe";
#endif
const QString IUpdaterComponent::X2GO = "x2go";
const QString IUpdaterComponent::VAGRANT = "vagrant";
const QString IUpdaterComponent::ORACLE_VIRTUALBOX = "oracle_virtualbox";
const QString IUpdaterComponent::VMWARE = "vmware";
const QString IUpdaterComponent::VMWARE_UTILITY = "vmware_utility";
const QString IUpdaterComponent::CHROME = "Chrome";
const QString IUpdaterComponent::FIREFOX = "Firefox";
const QString IUpdaterComponent::E2E = "e2e";
const QString IUpdaterComponent::VAGRANT_SUBUTAI = "vagrant_subutai";
const QString IUpdaterComponent::VAGRANT_VBGUEST = "vagrant_vbguest";
const QString IUpdaterComponent::VAGRANT_LIBVIRT = "vagrant_libvirt";
const QString IUpdaterComponent::VAGRANT_PARALLELS = "vagrant_parallels";
const QString IUpdaterComponent::VAGRANT_VMWARE_DESKTOP = "vagrant_vmware_desktop";
const QString IUpdaterComponent::SUBUTAI_BOX = "subutai_box";
const QString IUpdaterComponent::XQUARTZ = "xquartz";

const QString &
IUpdaterComponent::component_id_to_user_view(const QString& id) {
  static std::map<QString, QString> dct = {
    {P2P, "P2P"},
    {TRAY, "ControlCenter"},
    {X2GO, "X2Go-Client"},
    {VAGRANT, "Vagrant"},
    {ORACLE_VIRTUALBOX, "Oracle Virtualbox"},
    {CHROME, "Google Chrome"},
    {FIREFOX, "Mozilla Firefox"},
    {E2E, "Subutai E2E"},
    {VAGRANT_SUBUTAI, "Subutai plugin"},
    {VAGRANT_VBGUEST, "VirtualBox plugin"},
    {SUBUTAI_BOX, "Subutai box"},
    {XQUARTZ, "XQuartz"}
  };
  static const QString def = "";

  if (dct.find(id) == dct.end()) return def;
  return dct.at(id);
}

const QString &
IUpdaterComponent::component_id_changelog(const QString& id) {
  static std::map<QString, QString> dct_changelog = {
    {P2P, "https://github.com/subutai-io/p2p/releases/latest"},
    {TRAY, "https://github.com/subutai-io/control-center/releases/latest"},
    {X2GO, "https://wiki.x2go.org/doku.php/news:start"},
    {VAGRANT, "https://github.com/hashicorp/vagrant/blob/master/CHANGELOG.md"},
    {ORACLE_VIRTUALBOX, "https://www.virtualbox.org/wiki/Changelog"},
    {CHROME, "https://chromereleases.googleblog.com/"},
    {FIREFOX, "https://mozilla.org/en-US/firefox/releases/"},
    {E2E, "https://github.com/subutai-io/browser-plugins/releases/latest"},
    {VAGRANT_SUBUTAI, "https://github.com/subutai-io/vagrant/blob/master/CHANGELOG.md"},
    {VAGRANT_VBGUEST, "https://github.com/dotless-de/vagrant-vbguest/blob/master/CHANGELOG.md"},
    {SUBUTAI_BOX, "https://app.vagrantup.com/subutai/boxes/stretch"}
  };
  static const QString def = "";

  if (dct_changelog.find(id) == dct_changelog.end()) return def;
  return dct_changelog.at(id);
}

DlgNotification::NOTIFICATION_ACTION_TYPE
IUpdaterComponent::component_id_to_notification_action(const QString& id) {
  static std::map<QString, DlgNotification::NOTIFICATION_ACTION_TYPE> dct = {
    {P2P, DlgNotification::N_UPDATE_P2P},
    {TRAY, DlgNotification::N_UPDATE_TRAY},
    {X2GO, DlgNotification::N_ABOUT},
    {VAGRANT, DlgNotification::N_ABOUT},
    {ORACLE_VIRTUALBOX, DlgNotification::N_ABOUT},
    {CHROME, DlgNotification::N_ABOUT},
    {FIREFOX, DlgNotification::N_ABOUT},
    {VAGRANT_SUBUTAI, DlgNotification::N_ABOUT},
    {VAGRANT_VBGUEST, DlgNotification::N_ABOUT},
    {SUBUTAI_BOX, DlgNotification::N_ABOUT},
    {XQUARTZ, DlgNotification::N_ABOUT}
  };
  return dct.at(id);
}
