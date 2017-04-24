#include "NotificationObserver.h"
#include "SettingsManager.h"

void
CNotificationObserver::notify_all_internal(notification_level_t level,
                                           const QString &msg) {
  emit notify(level, msg);
}
////////////////////////////////////////////////////////////////////////////
