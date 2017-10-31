#include "tester.h"
#include "commonstest.h"
#include <QTest>

Tester::Tester () {
  // adding all tests here
  addTest(new CommonsTest);
}

Tester* Tester::Instance() {
    static Tester tester;
    return &tester;
}

Tester::TestList& Tester::testList() {
   static TestList list;
   return list;
}

void Tester::addTest(QObject* object) {
    TestList&  list = testList();
    list.append(object);
}

int Tester::runAllTest() {
    int ret = 0;
    foreach (QObject* test, testList())
        ret += QTest::qExec(test);
    return ret;
}
