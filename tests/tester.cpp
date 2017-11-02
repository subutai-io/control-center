#include <QTest>
#include "Tester.h"
#include "CCommonsTest.h"
#include "LanguageControllerTest.h"
#include "LoggerTest.h"
#include "SystemCallWrapperTest.h"
#include "DlgNotificationsModelTest.h"
#include "NotificationObserverTest.h"
#include "NotificationLoggerTest.h"

Tester::Tester () {
  /* add all tests here */
  addTest(new CommonsTest);
  addTest(new LanguageControllerTest);
  addTest(new SystemCallWrapperTest);
  addTest(new DlgNotificationsModelTest);
  addTest(new NotificationObserverTest);
  addTest(new NotificationLoggerTest);
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
void Tester::runLast() {
    TestList& list = testList();
    QTest::qExec(list.last());
}

int Tester::runAllTest() {
    runLast();

    //int ret = 0;
    //foreach (QObject* test, testList())
    //    ret += QTest::qExec(test);
    return 0;
}
