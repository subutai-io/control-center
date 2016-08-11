#include <QApplication>
#include <QDir>

#include "updater/HubComponentsUpdater.h"
#include "updater/ExecutableUpdater.h"
#include "DownloadFileManager.h"
#include "SystemCallWrapper.h"
#include "RestWorker.h"
#include "NotifiactionObserver.h"
#include "DownloadFileManager.h"
#include "libssh2/LibsshErrors.h"
#include "updater/UpdaterComponentP2P.h"
#include "updater/UpdaterComponentTray.h"
#include "updater/UpdaterComponentRH.h"

using namespace update_system;

CHubComponentsUpdater::CHubComponentsUpdater() {
  IUpdaterComponent *uc_tray, *uc_p2p, *uc_rh;
  uc_tray = new CUpdaterComponentTray;
  uc_p2p  = new CUpdaterComponentP2P;
  uc_rh   = new CUpdaterComponentRH;

  m_dct_components[IUpdaterComponent::TRAY] = CUpdaterComponentItem(uc_tray);
  m_dct_components[IUpdaterComponent::P2P]  = CUpdaterComponentItem(uc_p2p);
  m_dct_components[IUpdaterComponent::RH]   = CUpdaterComponentItem(uc_rh);

  connect(&m_dct_components[IUpdaterComponent::TRAY], SIGNAL(timer_timeout(const QString&)),
      this, SLOT(update_component_timer_timeout(const QString&)));
  connect(&m_dct_components[IUpdaterComponent::P2P], SIGNAL(timer_timeout(const QString&)),
      this, SLOT(update_component_timer_timeout(const QString&)));
  connect(&m_dct_components[IUpdaterComponent::RH], SIGNAL(timer_timeout(const QString&)),
      this, SLOT(update_component_timer_timeout(const QString&)));

  ///
  connect(uc_tray, SIGNAL(update_progress(QString,qint64,qint64)),
          this, SLOT(update_component_progress_sl(QString,qint64,qint64)));
  connect(uc_p2p, SIGNAL(update_progress(QString,qint64,qint64)),
          this, SLOT(update_component_progress_sl(QString,qint64,qint64)));
  connect(uc_rh, SIGNAL(update_progress(QString,qint64,qint64)),
          this, SLOT(update_component_progress_sl(QString,qint64,qint64)));

  connect(uc_tray, SIGNAL(update_finished(QString,bool)),
          this, SLOT(update_component_finished_sl(QString,bool)));
  connect(uc_p2p, SIGNAL(update_finished(QString,bool)),
          this, SLOT(update_component_finished_sl(QString,bool)));
  connect(uc_rh, SIGNAL(update_finished(QString,bool)),
          this, SLOT(update_component_finished_sl(QString,bool)));

  ///
  set_p2p_update_freq();
  set_rh_update_freq();
  set_tray_update_freq();
}

CHubComponentsUpdater::~CHubComponentsUpdater() {
  for(auto i = m_dct_components.begin(); i != m_dct_components.end(); ++i) {
    delete i->second.Component();
  }
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_update_freq(const QString &component_id,
                                       CSettingsManager::update_freq_t freq) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    CApplicationLog::Instance()->LogError(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }

  m_dct_components[component_id].timer_stop();
  if (freq == CSettingsManager::UF_NEVER)
    return;

  m_dct_components[component_id].set_timer_interval(
        CSettingsManager::update_freq_to_sec(freq)*1000);
  m_dct_components[component_id].timer_start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::update_component_timer_timeout(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    CApplicationLog::Instance()->LogError(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }

  m_dct_components[component_id].timer_stop();
  if (m_dct_components[component_id].Component()->update_available()) {
    if (m_dct_components[component_id].autoupdate) {
      CNotificationObserver::Instance()->NotifyAboutInfo(
            QString("%1 updating started").arg(component_id));
      m_dct_components[component_id].Component()->update();
    } else {
      CNotificationObserver::Instance()->NotifyAboutInfo(
            QString("New version of %1 is available!").arg(component_id));
    }
  }
  m_dct_components[component_id].timer_start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_p2p_update_freq() {
  set_update_freq(IUpdaterComponent::P2P, CSettingsManager::Instance().p2p_update_freq());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_rh_update_freq() {
  set_update_freq(IUpdaterComponent::RH, CSettingsManager::Instance().rh_update_freq());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_tray_update_freq() {
  set_update_freq(IUpdaterComponent::TRAY, CSettingsManager::Instance().tray_update_freq());
}
////////////////////////////////////////////////////////////////////////////

bool
CHubComponentsUpdater::is_update_available(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    CApplicationLog::Instance()->LogError(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return false;
  }
  return m_dct_components[component_id].Component()->update_available();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::force_update(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    CApplicationLog::Instance()->LogError(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }
  m_dct_components[component_id].Component()->update();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::update_component_progress_sl(QString file_id, qint64 cur,
                                                                qint64 full) {
  emit download_file_progress(file_id, cur, full);
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::update_component_finished_sl(QString file_id, bool replaced) {
  emit updating_finished(file_id, replaced);
}
////////////////////////////////////////////////////////////////////////////
