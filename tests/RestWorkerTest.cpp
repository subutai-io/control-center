#include "RestWorkerTest.h"
#include "RestWorker.h"
#include <QTest>
#include <QSignalSpy>
#include <QDebug>
#include <QTimer>

void RestWorkerTest::initTestCase(){
  try{
    int http_code, err_code, network_err;
    CRestWorker::Instance()->login("SEFI2@unist.ac.kr", "testing",
                                 http_code, err_code, network_err);
    if (err_code == RE_LOGIN_OR_EMAIL){
      throw 0;
    }
  }
  catch(...) {
    QVERIFY(false);
  }
}

void RestWorkerTest::test_update_balance() {
    try{
      QTimer timer;
      QEventLoop loop;
      QSignalSpy spyUpdate(CRestWorker::Instance(), &CRestWorker::on_get_balance_finished);
      CRestWorker::Instance()->update_balance();
      connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
      timer.start(5000);
      loop.exec();
      QCOMPARE(spyUpdate.count(), 1);
    }
    catch(...) {
      QVERIFY(false);
    }
}


////////////////////////////////////////////////////////

void RestWorkerTest::test_update_my_peers() {
    try{
      QTimer timer;
      QEventLoop loop;
      QSignalSpy spyUpdate(CRestWorker::Instance(), &CRestWorker::on_get_my_peers_finished);
      timer.setSingleShot(true);
      CRestWorker::Instance()->update_my_peers();
      connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
      timer.start(5000);
      loop.exec();
      QCOMPARE(spyUpdate.count(), 1);
    }
    catch(...) {
      QVERIFY(false);
    }
}

////////////////////////////////////////////////////////


void RestWorkerTest::test_update_environments() {
    try{
      QTimer timer;
      QEventLoop loop;
      QSignalSpy spyUpdate(CRestWorker::Instance(), &CRestWorker::on_get_environments_finished);
      timer.setSingleShot(true);
      CRestWorker::Instance()->update_environments();
      connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
      timer.start(5000);
      loop.exec();
      QCOMPARE(spyUpdate.count(), 1);
    }
    catch(...) {
      QVERIFY(false);
    }
}

////////////////////////////////////////////////////////


void RestWorkerTest::cleanupTestCase() {

}

