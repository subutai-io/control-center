#include "RestWorkerTest.h"
#include "RestWorker.h"

void RestWorkerTest::test_update_balance() {
  CRestWorker::Instance()->update_balance();
}

////////////////////////////////////////////////////////

void RestWorkerTest::test_update_my_peers() {
  CRestWorker::Instance()->update_my_peers();
}

////////////////////////////////////////////////////////


void RestWorkerTest::test_update_environments() {
  CRestWorker::Instance()->update_environments();
}

////////////////////////////////////////////////////////




