#ifndef RESTWORKERTEST_H
#define RESTWORKERTEST_H

#include <QObject>

class RestWorkerTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_update_balance();
    void test_update_environments();
    void test_update_my_peers();
    void cleanupTestCase();
};

#endif // RESTWORKERTEST_H
