#include "commonstest.h"
#include "Commons.h"

#include <QTest>

//////////////////////////////////////////////////////////////////////
void CommonsTest::testFileMd5_data() {
    QTest::addColumn<QString> ("file_path");
    QTest::addColumn<QString> ("expected_hash");
    QString file_path = ".";
    QString expected_hash = "something";
    QTest::newRow("absolute file path") << file_path << expected_hash;
}

void CommonsTest::testFileMd5() {
    QFETCH(QString , file_path);
    QFETCH(QString , expected_hash);
    QString returned_hash = CCommons::FileMd5(file_path);
    QCOMPARE(returned_hash , expected_hash);
}


//////////////////////////////////////////////////////////////////////
