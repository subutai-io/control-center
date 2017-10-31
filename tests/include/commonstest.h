#ifndef COMMONSTEST_H
#define COMMONSTEST_H

#include <QObject>

class CommonsTest : public QObject
{
Q_OBJECT

private slots:
    void testFileMd5();
    void testFileMd5_data();


};

#endif // COMMONSTEST_H
