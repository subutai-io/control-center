#include "RhControllerTest.h"
#include "RhController.h"
#include "SettingsManager.h"
#include <QTest>
#include <map>

void RhControllerTest::initTestCase() {
}

void RhControllerTest::testRefresh() {
   // CRhController *rhc = new CRhController();
    static const QString current_setting = "current_setting";
    static const QString default_setting = "default_setting";
    static const QString localhost = "127.0.0.1";
    static const QString rh_host = CSettingsManager::Instance().rh_host();
    //rhc->refresh();
    //for later ->>>>>
    //std::map<QString, QString> ::const_iterator it = rhc->dct_resource_hosts().find(default_setting);


   // QCOMPARE(*(rhc->dct_resource_hosts().find(current_setting)), rh_host);
    //QCOMPARE(*it, localhost);
    //QVERIFY (!rhc->dct_resource_hosts().empty());
    //QVERIFY (rhc->dct_resource_hosts().size() <= 2);
    //QVERIFY(rhc->m_refresh_in_progress == true);

    //rhc->deleteLater();
}

void
RhControllerTest::test_found_device_slot() {

  // CRhController *rhc = new CRhController();
  // rhc->found_device_slot("1" , "1");

  //if (!m_refresh_in_progress) return;
  //if (m_dct_resource_hosts[current_setting] == location) return;
  //m_has_changes |= (m_dct_resource_hosts.find(uid) == m_dct_resource_hosts.end());
  //m_dct_resource_hosts[uid] = location;

  //rhc->deleteLater();
}

void RhControllerTest::cleanupTestCase() {
}
