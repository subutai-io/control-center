#include "Tester.h"
#include "CCommonsTest.h"
#include "LanguageControllerTest.h"
#include <QTest>
#include "LoggerTest.h"

Tester::Tester () {
  // add all tests here
  //addTest(new CommonsTest);
  //addTest(new LanguageControllerTest);
  addTest(new LoggerTest);
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
