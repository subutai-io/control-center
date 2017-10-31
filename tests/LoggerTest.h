#ifndef LOGGERTEST_H
#define LOGGERTEST_H

#include <QObject>

class LoggerTest : public QObject
{
    Q_OBJECT

private slots:
    void testTypeToLevel();
    void testTypeToLevel_data();
};

#endif // LOGGERTEST_H
