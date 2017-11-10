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

static const QString current_setting = "current_setting";
static const QString default_setting = "default_setting";
static const QString localhost = "127.0.0.1";

void
CRhController::refresh() {
  m_dct_resource_hosts.clear();

  if (CSettingsManager::Instance().rh_host() != localhost)
    m_dct_resource_hosts[default_setting] = localhost;
  m_dct_resource_hosts[current_setting] = CSettingsManager::Instance().rh_host();

  CSsdpController::Instance()->search();
  m_refresh_in_progress = true;
  m_delay_timer.start();
}
////////////////////////////////////////////////////////////////////////////

void
CRhController::found_device_slot(QString uid, QString location) {
  if (!m_refresh_in_progress) return;
  if (m_dct_resource_hosts[current_setting] == location) return;
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
