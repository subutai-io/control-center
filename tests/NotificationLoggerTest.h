#ifndef NOTIFICATIONLOGGERTEST_H
#define NOTIFICATIONLOGGERTEST_H
#include <QObject>

class NotificationLoggerTest : public QObject
{
    Q_OBJECT
private slots:
    void testFromString();
    void testFromString_data();

};

#endif // NOTIFICATIONLOGGERTEST_H
