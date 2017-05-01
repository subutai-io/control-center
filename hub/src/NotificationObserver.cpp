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
////////////////////////////////////////////////////////////////////////////

void
CNotificationObserver::notify_all_internal(notification_level_t level,
                                           const QString &msg) {
  emit notify(level, msg);
}
////////////////////////////////////////////////////////////////////////////

