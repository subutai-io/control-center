#include <QtGui>
#include <QWidget>

#include "NotificationObserver.h"
#include "SettingsManager.h"

const QString &
CNotificationObserver::notification_level_to_str(notification_level_t nt) {
  static QString nt_str[] = {
    "Info", "Warning",
    "Error", "Critical error"
  };
  return nt_str[nt];
}

const QString &
CNotificationObserver::notifications_preffered_place_to_str(
    CNotificationObserver::notification_preffered_place_t pl) {
  static QString npp_str[] = {
    "Top-right corner", "Bottom-right corner",
    "Bottom-left corner", "Top-left corner"
  };
  return npp_str[pl];
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Error(const QString &msg) {
  Instance()->notify_all_internal(NL_ERROR, msg);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Info(const QString &msg) {
  Instance()->notify_all_internal(NL_INFO, msg);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Warning(const QString &msg) {
  Instance()->notify_all_internal(NL_WARNING, msg);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::Critical(const QString &msg) {
  Instance()->notify_all_internal(NL_CRITICAL, msg);
}
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::notify_all_internal(notification_level_t level,
                                           const QString &msg) {
  emit notify(level, msg);
}
////////////////////////////////////////////////////////////////////////////

