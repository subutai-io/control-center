#include "LoggerTest.h"
#include "Logger.h"
#include <QTest>


void LoggerTest::testTypeToLevel_data() {
    QTest::addColumn<int> ("type");
    QTest::addColumn<int>("expected_level");
    QTest::newRow("Debug Type") << (int)QtDebugMsg << (int) Logger::LOG_DEBUG;
    QTest::newRow("Info Type") << (int)QtInfoMsg << (int) Logger::LOG_INFO;
    QTest::newRow("Warning Type") << (int)QtWarningMsg << (int) Logger::LOG_WARNING;
}

void LoggerTest::testTypeToLevel() {
    //QFETCH(int, type);
    QFETCH(int, expected_level);
    int returned_value = 0; //Logger::typeToLevel((QtMsgType) type); temporary

    QCOMPARE(returned_value, expected_level);
}

////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
