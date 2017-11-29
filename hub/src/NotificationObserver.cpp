#include <QtGui>
#include <QWidget>

#include "NotificationObserver.h"
#include "SettingsManager.h"
#include "DlgNotification.h"

const QString &
CNotificationObserver::notification_level_to_str(notification_level_t nt) {
  static QString nt_str[] = {
    tr("Info"), tr("Warning"),
    tr("Error"), tr("Critical error")
  };
  return nt_str[nt];
}

const QString &
CNotificationObserver::notifications_preffered_place_to_str(
    CNotificationObserver::notification_preffered_place_t pl) {
  static QString npp_str[] = {
    tr("Top-right corner"), tr("Bottom-right corner"),
    tr("Bottom-left corner"), tr("Top-left corner")
  };
  return npp_str[pl];
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Error(const QString &msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  Instance()->notify_all_internal(NL_ERROR, msg, action_type);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Info(const QString &msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  Instance()->notify_all_internal(NL_INFO, msg, action_type);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Warning(const QString &msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  Instance()->notify_all_internal(NL_WARNING, msg, action_type);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Critical(const QString &msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  Instance()->notify_all_internal(NL_CRITICAL, msg, action_type);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::notify_all_internal(notification_level_t level,
                                           const QString &msg, DlgNotification::NOTIFICATION_ACTION_TYPE action_type) {
  emit notify(level, msg, action_type);
}
////////////////////////////////////////////////////////////////////////////

