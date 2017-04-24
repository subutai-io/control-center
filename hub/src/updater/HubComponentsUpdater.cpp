#include <QApplication>
#include <QDir>

#include "updater/HubComponentsUpdater.h"
#include "updater/ExecutableUpdater.h"
#include "DownloadFileManager.h"
#include "SystemCallWrapper.h"
#include "RestWorker.h"
#include "NotificationObserver.h"
#include "DownloadFileManager.h"
#include "updater/UpdaterComponentP2P.h"
#include "updater/UpdaterComponentTray.h"
#include "updater/UpdaterComponentRH.h"
#include "updater/UpdaterComponentRHManagement.h"

using namespace update_system;

CHubComponentsUpdater::CHubComponentsUpdater() {
  IUpdaterComponent *uc_tray, *uc_p2p, *uc_rh, *uc_rhm;
  uc_tray = new CUpdaterComponentTray;
  uc_p2p  = new CUpdaterComponentP2P;
  uc_rh   = new CUpdaterComponentRH;
  uc_rhm  = new CUpdaterComponentRHM;
  IUpdaterComponent* ucs[] = {uc_tray, uc_p2p, uc_rh, uc_rhm, NULL};

  m_dct_components[IUpdaterComponent::TRAY] = CUpdaterComponentItem(uc_tray);
  m_dct_components[IUpdaterComponent::P2P]  = CUpdaterComponentItem(uc_p2p);
  m_dct_components[IUpdaterComponent::RH]   = CUpdaterComponentItem(uc_rh);
  m_dct_components[IUpdaterComponent::RHMANAGEMENT] = CUpdaterComponentItem(uc_rhm);

  for(int i = 0; ucs[i] ;++i) {
    connect(&m_dct_components[ucs[i]->component_id()], SIGNAL(timer_timeout(const QString&)),
        this, SLOT(update_component_timer_timeout(const QString&)));
    connect(ucs[i], SIGNAL(update_progress(const QString&,qint64,qint64)),
        this, SLOT(update_component_progress_sl(const QString&,qint64,qint64)));
    connect(ucs[i], SIGNAL(update_finished(const QString&,bool)),
        this, SLOT(update_component_finished_sl(const QString&,bool)));
  }
  ///
  set_p2p_update_freq();
  set_rh_update_freq();
  set_tray_update_freq();
  set_rh_management_update_freq();
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
CHubComponentsUpdater::set_component_autoupdate(const QString &component_id,
                                                bool autoupdate) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    CApplicationLog::Instance()->LogError(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }
  m_dct_components[component_id].autoupdate = autoupdate;
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
      CNotificationObserver::Instance()->Info(
            QString("%1 updating started").arg(component_id));
      m_dct_components[component_id].Component()->update();
    } else {
      CNotificationObserver::Instance()->Info(
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

void
CHubComponentsUpdater::set_rh_management_update_freq() {
  set_update_freq(IUpdaterComponent::RHMANAGEMENT,
                  CSettingsManager::Instance().rh_management_update_freq());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_p2p_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::P2P,
                           CSettingsManager::Instance().p2p_autoupdate());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_rh_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::RH,
                           CSettingsManager::Instance().rh_autoupdate());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_tray_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::TRAY,
                           CSettingsManager::Instance().tray_autoupdate());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_rh_management_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::RHMANAGEMENT,
                           CSettingsManager::Instance().rh_management_autoupdate());
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
CHubComponentsUpdater::update_component_progress_sl(const QString& file_id, qint64 cur,
                                                    qint64 full) {
  emit download_file_progress(file_id, cur, full);
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::update_component_finished_sl(const QString& file_id, bool replaced) {
  emit updating_finished(file_id, replaced);
}
////////////////////////////////////////////////////////////////////////////
