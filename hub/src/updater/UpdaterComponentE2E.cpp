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
#include "updater/ExecutableUpdater.h"
#include "updater/HubComponentsUpdater.h"
#include "updater/UpdaterComponentE2E.h"

CUpdaterComponentE2E::CUpdaterComponentE2E() { m_component_id = E2E; }

CUpdaterComponentE2E::~CUpdaterComponentE2E() {}

QString CUpdaterComponentE2E::download_e2e_path() {
  QStringList lst_temp =
      QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  return (lst_temp.isEmpty() ? QApplication::applicationDirPath()
                             : lst_temp[0]);
}

bool CUpdaterComponentE2E::update_available_internal() {
  QString version;
  CSystemCallWrapper::subutai_e2e_version(version);
  return version == "undefined";
}

chue_t CUpdaterComponentE2E::install_internal() {
  qDebug() << "Starting install subutai e2e";
  if (CSettingsManager::Instance().default_browser() == "Chrome") {
    QMessageBox *msg_box = new QMessageBox(
        QMessageBox::Information, QObject::tr("Attention!"),
        QObject::tr(
            "The <a href='https://subutai.io/getting-started.html#E2E'>Subutai "
            "E2E plugin</a>"
            " manages your PGP keys.<br>"
            "Installing the E2E plugin will restart your browser. "
            "Be sure to save your work before installing, and "
            "approve the extension after installing.<br>"
            "Do you want to proceed?"),
        QMessageBox::Yes | QMessageBox::No);
    msg_box->setTextFormat(Qt::RichText);

    QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                     &QMessageBox::deleteLater);
    if (msg_box->exec() != QMessageBox::Yes) {
      install_finished_sl(false);
      return CHUE_SUCCESS;
    }
  }
  static QString empty_stings = "";
  SilentInstaller *silent_installer = new SilentInstaller(this);
  silent_installer->init(empty_stings, empty_stings, CC_E2E);
  connect(silent_installer, &SilentInstaller::outputReceived, this,
          &CUpdaterComponentE2E::install_finished_sl);
  silent_installer->startWork();
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentE2E::update_internal() {
  update_finished_sl(true);
  return CHUE_SUCCESS;
}

chue_t CUpdaterComponentE2E::uninstall_internal() {
  qDebug() << "Starting uninstall subutai e2e";
  static QString empty_string = "";
  SilentUninstaller *silent_uninstaller = new SilentUninstaller(this);
  silent_uninstaller->init(empty_string, empty_string, CC_E2E);
  connect(silent_uninstaller, &SilentUninstaller::outputReceived, this,
          &CUpdaterComponentE2E::uninstall_finished_sl);
  silent_uninstaller->startWork();
  return CHUE_SUCCESS;
}

void CUpdaterComponentE2E::update_post_action(bool success) {
  UNUSED_ARG(success);
}

void CUpdaterComponentE2E::install_post_interntal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to complete E2E plugin installation. You may try installing "
           "directly through the link under "
           "<a href='https://subutai.io/getting-started.html#E2E'>Getting "
           "Started</a>"
           " or try again by restarting the Control Center first."),
        DlgNotification::N_NO_ACTION);
    return;
  }
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr(
          "<br>Subutai E2E has been installed to your browser</br>"
          "<br>If E2E does not appear, please approve installation from "
          "chrome://extensions.\n\t</br>"
          "<br><a "
          "href='https://docs.subutai.io/Products/Bazaar/27_E2E_plugin.html'>"
          "Learn more about Subutai E2E."
          "</a></br>"),
      QMessageBox::Ok);
  msg_box->setTextFormat(Qt::RichText);
  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() == QMessageBox::Ok) {
    CSystemCallWrapper::chrome_last_section();
  }
}

void CUpdaterComponentE2E::uninstall_post_internal(bool success) {
  if (!success) {
    CNotificationObserver::Instance()->Info(
        tr("Failed to uninstall Subutai E2E plugin. Try uninstalling "
           "manually."),
        DlgNotification::N_NO_ACTION);
    return;
  }
  QMessageBox *msg_box = new QMessageBox(
      QMessageBox::Information, QObject::tr("Attention!"),
      QObject::tr("Subutai E2E has been uninstalled from your browser<br>"
                  "Press OK to restore your browser session."),
      QMessageBox::Ok);
  msg_box->setTextFormat(Qt::RichText);
  QObject::connect(msg_box, &QMessageBox::finished, msg_box,
                   &QMessageBox::deleteLater);
  if (msg_box->exec() == QMessageBox::Ok) {
    CSystemCallWrapper::chrome_last_section();
  }
}
