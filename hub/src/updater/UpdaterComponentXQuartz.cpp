#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

#include "Commons.h"
#include "DownloadFileManager.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "P2PController.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentXQuartz.h"

CUpdaterComponentXQuartz::CUpdaterComponentXQuartz() { m_component_id = XQUARTZ; }

CUpdaterComponentXQuartz::~CUpdaterComponentXQuartz() {}

QString CUpdaterComponentXQuartz::download_xquartz_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentXQuartz::update_available_internal() {
  return true;
  // need to implement
}

chue_t CUpdaterComponentXQuartz::install_internal() {
  qDebug() << "Starting install xquart";
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentXQuartz::update_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentXQuartz::uninstall_internal() {
  update_progress_sl(100, 100);
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

void CUpdaterComponentXQuartz::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentXQuartz::install_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete XQuartz installation. Try again later, "
           "or install it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(
        tr("The XQuartz has been installed."),
        DlgNotification::N_NO_ACTION);
}

void CUpdaterComponentXQuartz::uninstall_post_internal(bool success) {
  if (!success)
    CNotificationObserver::Instance()->Error(
        tr("Failed to complete XQuartz uninstallation. Try again later, "
           "or uninstall it manually."),
        DlgNotification::N_NO_ACTION);
  else
    CNotificationObserver::Instance()->Info(tr("XQuartz has been uninstalled"),
                                            DlgNotification::N_NO_ACTION);
}
