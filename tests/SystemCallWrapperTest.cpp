#include "SystemCallWrapperTest.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"
#include <QTest>

void SystemCallWrapperTest::testSsystem() {
    QFETCH(QString , cmd);
    QFETCH(QStringList , args);
    QFETCH(QStringList , expected_output);
    QStringList returned_value = CSystemCallWrapper::ssystem(cmd, args, true, true , 5000).out;
    QCOMPARE(returned_value, expected_output);
}


void SystemCallWrapperTest::testSsystem_data() {
    QTest::addColumn<QString>("cmd");
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expected_output");
    QString cmd;
    QStringList args;
    QStringList expected_output;
    cmd = CSettingsManager::Instance().p2p_path();
    args << "-v";
    expected_output << "p2p version 6.1.9-dev";
    QTest::newRow("p2p test") << cmd << args << expected_output;
}

