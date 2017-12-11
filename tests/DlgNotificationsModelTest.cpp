#include "DlgNotificationsModelTest.h"
#include "DlgNotificationsModel.h"
#include <QTest>

void DlgNotificationsModelTest::test_sort_notification_by_date() {
    QFETCH(int , something);
    QCOMPARE(something , 3);
}

void DlgNotificationsModelTest::test_sort_notification_by_date_data() {
    QTest::addColumn<int>("something");
    QTest::addColumn<int>("anotherthing");
    QTest::newRow("test") << 1 << 2;

}
