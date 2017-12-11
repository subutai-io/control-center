#ifndef RHCONTROLLERTEST_H
#define RHCONTROLLERTEST_H

#include <QObject>
class CRhController;

class RhControllerTest : public QObject
{
    Q_OBJECT
private:
private slots:
    void initTestCase();
    // CRhController::refresh
    void testRefresh();

    // CRhController::found_device_slot
    void test_found_device_slot();


    void cleanupTestCase();
};

#endif // RHCONTROLLERTEST_H
