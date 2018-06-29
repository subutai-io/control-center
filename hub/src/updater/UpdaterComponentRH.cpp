#include "updater/IUpdaterComponent.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"
#include "SettingsManager.h"
#include "updater/UpdaterComponentRH.h"
#include "libssh2/include/LibsshController.h"
#include "Commons.h"

using namespace update_system;

CUpdaterComponentRH::CUpdaterComponentRH() {
  m_component_id = RH;
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
  qDebug() << "Starting to update RH";
  int exit_code = 0;
  system_call_wrapper_error_t res = CSystemCallWrapper::run_rh_updater(
                                    CSettingsManager::Instance().rh_host().toStdString().c_str(),
                                    CSettingsManager::Instance().rh_port(),
                                    CSettingsManager::Instance().rh_user().toStdString().c_str(),
                                    CSettingsManager::Instance().rh_pass().toStdString().c_str(),
                                    exit_code);

  if (res == SCWE_SUCCESS && exit_code == 0) {
    static const char* msg = "Resource host update succesfully finished";
    CNotificationObserver::Info(tr(msg), DlgNotification::N_NO_ACTION);
    qCritical("%s", msg);
    update_finished_sl(true);
    return CHUE_SUCCESS;
  }

  QString err_msg = tr("Resource host update failed with exit code : %1, call result : %2").
                    arg(exit_code).arg(CSystemCallWrapper::scwe_error_to_str(res));
  CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
  qCritical("%s", err_msg.toStdString().c_str());
  update_finished_sl(false);
  return CHUE_FAILED;
}
////////////////////////////////////////////////////////////////////////////

void
CUpdaterComponentRH::update_post_action(bool success) {
  UNUSED_ARG(success); //do nothing
}
//////////////////////////////////////////////////////////////////////////////
/// instalation staff just to make compiler happy
chue_t CUpdaterComponentRH::install_internal() {
  return CHUE_SUCCESS;
}

void CUpdaterComponentRH::install_post_interntal(bool success) {
  UNUSED_ARG(success);
  return;
}

chue_t CUpdaterComponentRH::uninstall_internal() {
  return CHUE_SUCCESS;
}

void CUpdaterComponentRH::uninstall_post_internal(bool success) {}

