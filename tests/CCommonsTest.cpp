#include "CCommonsTest.h"
#include "Commons.h"
#include <QTest>

//////////////////////////////////////////////////////////////////////
void CommonsTest::testFileMd5_data() {
    QTest::addColumn<QString> ("file_path");
    QTest::addColumn<QString> ("expected_hash");
    QString file_path = ".";
    QString expected_hash = "";
    QTest::newRow("absolute file path") << file_path << expected_hash;
}

void CommonsTest::testFileMd5() {
    QFETCH(QString , file_path);
    QFETCH(QString , expected_hash);
    QString returned_value = CCommons::FileMd5(file_path);
    QCOMPARE(returned_value , expected_hash);
}
//////////////////////////////////////////////////////////////////////
void CommonsTest::testNetworkErrorToString_data(){
    QTest::addColumn<int> ("err_code");
    QTest::addColumn<QString> ("err_string");
    QTest::newRow("Temporary Network Failure Error Test") << 7 << "Temporary Network Failure Error";
    QTest::newRow("Unknown Content Error Test") << 299 << "Unknown Content Error";
}

void CommonsTest::testNetworkErrorToString(){
    QFETCH(int , err_code);
    QFETCH(QString, err_string);
    QString returned_value = CCommons::NetworkErrorToString(err_code);
    QCOMPARE(returned_value , err_string);
}

//////////////////////////////////////////////////////////////////////

void CommonsTest::testIsApplicationLaunchable_data(){
    QTest::addColumn<QString> ("file_path");
    QTest::addColumn<bool> ("yes_no");
    QString launchanble_path;
    QTest::newRow("Launchable") << launchanble_path << false; // temporary
    QTest::newRow("Launchable") << "/some_path.exe" << false;

}

void CommonsTest::testIsApplicationLaunchable(){
    QFETCH(QString , file_path);
    QFETCH(bool, yes_no);
    bool returned_value = CCommons::IsApplicationLaunchable(file_path);
    QCOMPARE(returned_value , yes_no);
}

//////////////////////////////////////////////////////////////////////


void CommonsTest::testDefaultTerminals() {
//    QCOMPARE(QStringList("") , CCommons::DefaultTerminals());
}
//////////////////////////////////////////////////////////////////////
void CommonsTest::testHasRecommendedTerminalArg_data(){

}
void CommonsTest::testHasRecommendedTerminalArg(){

}
//////////////////////////////////////////////////////////////////////

