#include <QTest>
#include "Tester.h"
#include "CCommonsTest.h"
#include "LanguageControllerTest.h"
#include "LoggerTest.h"
#include "SystemCallWrapperTest.h"
#include "DlgNotificationsModelTest.h"
#include "NotificationObserverTest.h"
#include "NotificationLoggerTest.h"
#include "SettingsManagerTest.h"
#include "RhControllerTest.h"

Tester::Tester () {
  /* add all tests here */
  addTest(new CommonsTest);
  addTest(new LanguageControllerTest);
  addTest(new DlgNotificationsModelTest);
  addTest(new NotificationObserverTest);
  addTest(new NotificationLoggerTest);
  addTest(new SystemCallWrapperTest);
  addTest(new SettingsManagerTest);
  addTest(new RhControllerTest);
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

void Tester::runAllTest() {
    foreach (QObject* test, testList())
        QTest::qExec(test);
}
