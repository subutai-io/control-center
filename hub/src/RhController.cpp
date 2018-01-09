#include "RhController.h"
#include "SsdpController.h"
#include "SettingsManager.h"

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
#include "SystemCallWrapper.h"

void ssh_to_rh(const QString &peer_fingerprint) {
  QString ip;
  QString port;
  QString user;
  QString pass;

  system_call_wrapper_error_t err = CSystemCallWrapper::run_ssh_in_terminal(ip, port, user, pass);

}
////////////////////////////////////////////////////////////////////////////
