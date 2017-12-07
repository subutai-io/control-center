#ifndef SYSTEMCALLWRAPPERTEST_H
#define SYSTEMCALLWRAPPERTEST_H

#include <QObject>

class SystemCallWrapperTest : public QObject
{
    Q_OBJECT

private slots:
    // CSystemCallWrapper::ssystem
    void testSsystem();
    void testSsystem_data();

    //CSystemCallWrapper::is_in_swarm
    //void testIsInSwarm();
    //void testIsInSwarm_data();


public slots:
};

#endif // SYSTEMCALLWRAPPERTEST_H
