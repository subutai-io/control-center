#include "RhController.h"
#include "SsdpController.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include <QPushButton>
#include "NotificationObserver.h"


CRhController::CRhController(QObject *parent) :
  QObject(parent),
  m_has_changes(false),
  m_refresh_in_progress(false) {

  m_refresh_timer.setInterval(60*1000);
  m_delay_timer.setInterval(REFRESH_DELAY_SEC*1000); //ssdp should use 5 seconds. BUT we will give 1 extra second :)

  connect(CSsdpController::Instance(), &CSsdpController::found_device,
          this, &CRhController::found_device_slot);
  connect(&m_refresh_timer, &QTimer::timeout,
          this, &CRhController::refresh_timer_timeout);
  connect(&m_delay_timer, &QTimer::timeout,
          this, &CRhController::delay_timer_timeout);
  m_refresh_timer.start();
}

CRhController::~CRhController() {
}

void
CRhController::init() {
  refresh();
}
////////////////////////////////////////////////////////////////////////////


void
CRhController::refresh() {
  m_dct_resource_hosts.clear();
  CSsdpController::Instance()->search();
  m_refresh_in_progress = true;
  m_delay_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
CRhController::found_device_slot(QString uid, QString location) {
  if (!m_refresh_in_progress) return;
  m_has_changes |= (m_dct_resource_hosts.find(uid) == m_dct_resource_hosts.end());
  m_dct_resource_hosts[uid] = location;
}
////////////////////////////////////////////////////////////////////////////

void
CRhController::refresh_timer_timeout() {
  refresh();
}
////////////////////////////////////////////////////////////////////////////

void
CRhController::delay_timer_timeout() {
  m_refresh_in_progress = false;
  m_delay_timer.stop();
  emit resource_host_list_updated(m_has_changes);
  m_has_changes = false;
}
////////////////////////////////////////////////////////////////////////////
#include <QtConcurrent/QtConcurrent>
#include "LibsshController.h"

void CRhController::ssh_to_rh(const QString &peer_fingerprint) {
  QString ip = CSettingsManager::Instance().rh_host(peer_fingerprint);
  QString port = QString::number(CSettingsManager::Instance().rh_port(peer_fingerprint));
  QString user = CSettingsManager::Instance().rh_user(peer_fingerprint);
  QString pass = CSettingsManager::Instance().rh_pass(peer_fingerprint);

  int *exit_code = new int;
  *exit_code = 0;

  QFuture<system_call_wrapper_error_t> res =
      QtConcurrent::run(CSystemCallWrapper::is_peer_available, peer_fingerprint, exit_code);
  res.waitForFinished();

  if (res.result() == SCWE_SUCCESS) {
    res = QtConcurrent::run(CSystemCallWrapper::run_sshpass_in_terminal, user, ip, port, pass);
    res.waitForFinished();
  }

  emit ssh_to_rh_finished(peer_fingerprint, res.result(), *exit_code);
  delete exit_code;
}

////////////////////////////////////////////////////////////////////////////
