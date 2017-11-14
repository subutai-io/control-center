#include "updater/IUpdaterComponent.h"
#include "DlgNotification.h"

#include <map>

using namespace update_system;

#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::P2P = "p2p";
#else
const QString IUpdaterComponent::P2P = "p2p.exe";
#endif
#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::TRAY = "SubutaiTray";
#else
const QString IUpdaterComponent::TRAY = "SubutaiTray.exe";
#endif

const QString IUpdaterComponent::RH = "resource_host";
const QString IUpdaterComponent::RHMANAGEMENT = "resource_host_management";

const QString &
IUpdaterComponent::component_id_to_user_view(const QString& id) {
  static std::map<QString, QString> dct = {
    {P2P, "p2p"},
    {TRAY, "tray application"},
    {RH, "resource host"},
    {RHMANAGEMENT, "resource host management"}
  };
  static const QString def = "";

  if (dct.find(id) == dct.end()) return def;
  return dct.at(id);
}

DlgNotification::NOTIFICATION_ACTION_TYPE
IUpdaterComponent::component_id_to_notification_action(const QString& id) {
  static std::map<QString, DlgNotification::NOTIFICATION_ACTION_TYPE> dct = {
    {P2P, DlgNotification::N_UPDATE_P2P},
    {TRAY, DlgNotification::N_UPDATE_TRAY},
    {RH, DlgNotification::N_UPDATE_RH},
    {RHMANAGEMENT, DlgNotification::N_UPDATE_RHM}
  };
  return dct.at(id);
}
