#include "NotificationLoggerTest.h"
#include "NotificationLogger.h"
#include <QTest>

void NotificationLoggerTest::testFromString_data() {
    QTest::addColumn<QString>("input_string");
    QTest::addColumn<QDateTime>("expected_time");
    QTest::addColumn<QString>("expected_level");
    QTest::addColumn<QString>("expected_msg");
    QTest::addColumn<bool>("expected_converted");
    /* #1 test */
    QTest::newRow("Empty input") << "" << QDateTime::fromString("") << "" << "" << false;

    /* #2 test */
    QTest::newRow("OK input with all parts")
            << "1970-07-18T14:15:09###Info###Hello world"
            << QDateTime::fromString("1970-07-18T14:15:09" , Qt::ISODate)
            << "Info"
            << "Hello world" << true;



    /* #3 test */
    QTest::newRow("Almost valid, but invalid case")
            << "1970-07-18T14:15:###Info###Hello world"
            << QDateTime::fromString("1970-07-18T14:15:" , Qt::ISODate)
            << ""
            << "" << false;
}


void NotificationLoggerTest::testFromString() {
    QFETCH(QString, input_string);
    QFETCH(QDateTime, expected_time);
    QFETCH(QString, expected_level);
    QFETCH(QString, expected_msg);
    QFETCH(bool, expected_converted);

    bool converted;
    CNotification returned_value =
            CNotification::fromString(input_string, converted);

    QCOMPARE(returned_value.date_time(), expected_time);
    QCOMPARE(returned_value.level_str(), expected_level);
    QCOMPARE(returned_value.message(), expected_msg);
    QCOMPARE(converted, expected_converted);
}

/////////////////////////////////////////////////////////////////////////////












