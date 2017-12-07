#ifndef NOTIFICATIONOBSERVERTEST_H
#define NOTIFICATIONOBSERVERTEST_H

#include <QObject>

class NotificationObserverTest : public QObject
{
    Q_OBJECT
public slots:
    // CNotificationObserver::notification_level_to_str(notification_level_t nt)
    void test_notification_level_to_str_data();
    void test_notification_level_to_str();
    // CNotificationObserver::notifications_preffered_place_to_str
    void test_notifications_preffered_place_to_str_data();
    void test_notifications_preffered_place_to_str();



};

#endif // NOTIFICATIONOBSERVERTEST_H
