#include "NotificationObserverTest.h"
#include "NotificationObserver.h"
#include <QTest>


/////////////////////////////////////////////////////////////////////////////

void NotificationObserverTest::test_notification_level_to_str_data() {
    QTest::addColumn<int> ("notification_level");
    QTest::addColumn<QString> ("expected_string");
    QTest::newRow("Info level") << (int) CNotificationObserver::NL_INFO << "Info" << "\n";
    QTest::newRow("Warning level") << (int) CNotificationObserver::NL_WARNING << "Warning" << "\n";
}

void NotificationObserverTest::test_notification_level_to_str() {
    QFETCH(int, notification_level);
    QFETCH(QString, expected_string);
    QString returned_value =
            CNotificationObserver::notification_level_to_str(
                (CNotificationObserver::notification_level_t)notification_level);
    QCOMPARE(returned_value, expected_string);
}


/////////////////////////////////////////////////////////////////////////////

void NotificationObserverTest::test_notifications_preffered_place_to_str_data(){
    QTest::addColumn<int> ("notification_preffered_place");
    QTest::addColumn<QString> ("expected_string");
    QTest::newRow("bottom-left") << (int) CNotificationObserver::NPP_LEFT_DOWN << "Bottom-left corner" << "\n";
    QTest::newRow("Bottom right") << (int) CNotificationObserver::NPP_RIGHT_DOWN << "Bottom-right corner" << "\n";
}

void NotificationObserverTest::test_notifications_preffered_place_to_str(){
    QFETCH(int, notification_preffered_place);
    QFETCH(QString, expected_string);
    QString returned_value =
            CNotificationObserver::notifications_preffered_place_to_str(
                (CNotificationObserver::notification_preffered_place_t)notification_preffered_place);
    QCOMPARE(returned_value, expected_string);
}


/////////////////////////////////////////////////////////////////////////////



