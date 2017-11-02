#include "SystemCallWrapperTest.h"
#include "SystemCallWrapper.h"
#include <QTest>

void SystemCallWrapperTest::testSsystem() {
    QFETCH(QString , cmd);
    QFETCH(QStringList , args);
    QFETCH(QStringList , expected_output);
    QStringList returned_value = CSystemCallWrapper::ssystem(cmd, args, true, true , 5000).out;
    QCOMPARE(expected_output , returned_value);
}


void SystemCallWrapperTest::testSsystem_data() {
    QTest::addColumn<QString>("cmd");
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expected_output");
    QString cmd;
    QStringList args;
    QStringList expected_output;
    cmd = "C:\\WINDOWS\\system32\\cmd.exe";
    args << "ver";
    expected_output << "Microsoft Windows [Version 10.0.15063]";
    QTest::newRow("terminal test") << cmd << args << expected_output;
}

