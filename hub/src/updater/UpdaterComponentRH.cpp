#include "updater/IUpdaterComponent.h"
#include "SystemCallWrapper.h"
#include "NotifiactionObserver.h"
#include "ApplicationLog.h"
#include "libssh2/LibsshErrors.h"
#include "SettingsManager.h"
#include "updater/UpdaterComponentRH.h"

using namespace update_system;

CUpdaterComponentRH::CUpdaterComponentRH() {
  m_component_id = QString("resource_host");
}

CUpdaterComponentRH::~CUpdaterComponentRH() {

}
////////////////////////////////////////////////////////////////////////////

bool
CUpdaterComponentRH::update_available_internal() {
  bool result = false;
  CSystemCallWrapper::is_rh_update_available(result);
  return result;
}
////////////////////////////////////////////////////////////////////////////

chue_t
CUpdaterComponentRH::update_internal() {
  int exit_code = 0;
  CSystemCallWrapper::run_ss_updater(CSettingsManager::Instance().rh_host().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_port().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_user().toStdString().c_str(),
                                     CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                                     exit_code);

  if (exit_code == RLE_SUCCESS) {
    static const char* msg = "Resource host update succesfull finished";
    CNotificationObserver::NotifyAboutInfo(msg);
    CApplicationLog::Instance()->LogInfo(msg);
    return CHUE_SUCCESS;
  }

  QString err_msg = QString("Resource host update failed with exit code : %1").arg(exit_code);
  CNotificationObserver::NotifyAboutError(err_msg);
  CApplicationLog::Instance()->LogError(err_msg.toStdString().c_str());
  return CHUE_FAILED;
}
////////////////////////////////////////////////////////////////////////////
