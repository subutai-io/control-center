#ifndef RESTWORKERTEST_H
#define RESTWORKERTEST_H

#include <QObject>

class RestWorkerTest : public QObject
{
    Q_OBJECT
private slots:
    void test_update_balance();
    void test_update_environments();
    void test_update_my_peers();
};

#endif // RESTWORKERTEST_H
