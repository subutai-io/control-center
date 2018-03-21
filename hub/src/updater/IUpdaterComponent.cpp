#include "updater/IUpdaterComponent.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgNotification.h"
#include "SettingsManager.h"
#include "Commons.h"
#include "NotificationObserver.h"
#include <QStandardPaths>
#include <QApplication>
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "DownloadFileManager.h"

#include <map>

using namespace update_system;

#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::P2P = "P2P";
#else
const QString IUpdaterComponent::P2P = "p2p.exe";
#endif
#ifndef RT_OS_WINDOWS
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter";
#else
const QString IUpdaterComponent::TRAY = "SubutaiControlCenter.exe";
#endif

const QString IUpdaterComponent::RH = "resource_host";
const QString IUpdaterComponent::RHMANAGEMENT = "resource_host_management";
const QString IUpdaterComponent::X2GO = "x2go";
const QString IUpdaterComponent::VAGRANT = "vagrant";
const QString IUpdaterComponent::ORACLE_VIRTUALBOX = "oracle_virtualbox";

const QString &
IUpdaterComponent::component_id_to_user_view(const QString& id) {
  static std::map<QString, QString> dct = {
    {P2P, "P2P"},
    {TRAY, "ControlCenter"},
    {RH, "resource host"},
    {RHMANAGEMENT, "resource host management"},
    {X2GO, "X2Go-Client"},
    {VAGRANT, "Vagrant"},
    {ORACLE_VIRTUALBOX, "Oralce Virtualbox"}
  };
  static const QString def = "";

  if (dct.find(id) == dct.end()) return def;
  return dct.at(id);
}

DlgNotification::NOTIFICATION_ACTION_TYPE
IUpdaterComponent::component_id_to_notification_action(const QString& id) {
  static std::map<QString, DlgNotification::NOTIFICATION_ACTION_TYPE> dct = {
    {P2P, DlgNotification::N_UPDATE_P2P},
    {TRAY, DlgNotification::N_UPDATE_TRAY},
    {RH, DlgNotification::N_UPDATE_RH},
    {RHMANAGEMENT, DlgNotification::N_UPDATE_RHM},
    {X2GO, DlgNotification::N_ABOUT},
    {VAGRANT, DlgNotification::N_ABOUT},
    {ORACLE_VIRTUALBOX, DlgNotification::N_ABOUT}
  };
  return dct.at(id);
}


// declaration of some staff will be left here for some time
CUpdaterComponentX2GO::CUpdaterComponentX2GO() {
  m_component_id = X2GO;
}
CUpdaterComponentX2GO::~CUpdaterComponentX2GO() {
}
//////////////////////////////////////////////////////////
QString CUpdaterComponentX2GO::download_x2go_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
//////////////////////////////////////////////////////////
bool CUpdaterComponentX2GO::update_available_internal(){
    QString version;
    CSystemCallWrapper::x2go_version(version);
    return version == "undefined";
}
//////////////////////////////////////////////////////////////
chue_t CUpdaterComponentX2GO::install_internal(){
    qDebug()
            << "Starting install x2go";

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString file_name = x2go_kurjun_package_name();
    QString file_dir = download_x2go_path();
    QString str_x2go_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_x2go_downloaded_path,
                                                        item->size());

    SilentPackageInstallerX2GO *silent_installer = new SilentPackageInstallerX2GO(this);
    silent_installer->init(file_dir, file_name);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentPackageInstallerX2GO::outputReceived,
            this, &CUpdaterComponentX2GO::install_finished_sl);
    connect(silent_installer, &SilentPackageInstallerX2GO::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
///////////////////////////////////////////////////////////////
chue_t CUpdaterComponentX2GO::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////
void CUpdaterComponentX2GO::update_post_action(bool success){
    UNUSED_ARG(success);
}
/////////////////////////////////////////////////////////////////
void CUpdaterComponentX2GO::install_post_interntal(bool success){
    if(!success)
        CNotificationObserver::Instance()->Error(tr("x2go-client installation failed"), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("x2go-client has been installed."), DlgNotification::N_NO_ACTION);
}

/////////////////////////////////////////////////////////////////

CUpdaterComponentVAGRANT::CUpdaterComponentVAGRANT() {
  m_component_id = VAGRANT;
}
CUpdaterComponentVAGRANT::~CUpdaterComponentVAGRANT() {
}
//////////////////////////////////////////////////////////
QString CUpdaterComponentVAGRANT::download_vagrant_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
//////////////////////////////////////////////////////////
bool CUpdaterComponentVAGRANT::update_available_internal(){
    QString version;
    CSystemCallWrapper::vagrant_version(version);
    return version == "undefined";
}
//////////////////////////////////////////////////////////////
chue_t CUpdaterComponentVAGRANT::install_internal(){
    qDebug()
            << "Starting install vagrant";

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString file_name = vagrant_kurjun_package_name();
    QString file_dir = download_vagrant_path();
    QString str_vagrant_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_vagrant_downloaded_path,
                                                        item->size());

    SilentPackageInstallerVAGRANT *silent_installer = new SilentPackageInstallerVAGRANT(this);
    silent_installer->init(file_dir, file_name);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentPackageInstallerVAGRANT::outputReceived,
            this, &CUpdaterComponentVAGRANT::install_finished_sl);
    connect(silent_installer, &SilentPackageInstallerVAGRANT::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
///////////////////////////////////////////////////////////////
chue_t CUpdaterComponentVAGRANT::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////
void CUpdaterComponentVAGRANT::update_post_action(bool success){
    UNUSED_ARG(success);
}
/////////////////////////////////////////////////////////////////
void CUpdaterComponentVAGRANT::install_post_interntal(bool success){
    if(success) CNotificationObserver::Instance()->Info(tr("vagrant has been installed."), DlgNotification::N_NO_ACTION);
}
//////////////////////////////////////////////////////////////////

CUpdaterComponentORACLE_VIRTUALBOX::CUpdaterComponentORACLE_VIRTUALBOX() {
  m_component_id = ORACLE_VIRTUALBOX;
}
CUpdaterComponentORACLE_VIRTUALBOX::~CUpdaterComponentORACLE_VIRTUALBOX() {
}
//////////////////////////////////////////////////////////
QString CUpdaterComponentORACLE_VIRTUALBOX::download_oracle_virtualbox_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
//////////////////////////////////////////////////////////
bool CUpdaterComponentORACLE_VIRTUALBOX::update_available_internal(){
    QString version;
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version == "undefined";
}
//////////////////////////////////////////////////////////////
chue_t CUpdaterComponentORACLE_VIRTUALBOX::install_internal(){
    qDebug()
            << "Starting install oracle virtualbox";

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString file_name = oracle_virtualbox_kurjun_package_name();
    QString file_dir = download_oracle_virtualbox_path();
    QString str_oracle_virtualbox_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_oracle_virtualbox_downloaded_path,
                                                        item->size());

    SilentPackageInstallerORACLE_VIRTUALBOX *silent_installer = new SilentPackageInstallerORACLE_VIRTUALBOX(this);
    silent_installer->init(file_dir, file_name);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation script."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentPackageInstallerORACLE_VIRTUALBOX::outputReceived,
            this, &CUpdaterComponentORACLE_VIRTUALBOX::install_finished_sl);
    connect(silent_installer, &SilentPackageInstallerORACLE_VIRTUALBOX::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
///////////////////////////////////////////////////////////////
chue_t CUpdaterComponentORACLE_VIRTUALBOX::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
////////////////////////////////////////////////////////////////
void CUpdaterComponentORACLE_VIRTUALBOX::update_post_action(bool success){
    UNUSED_ARG(success);
}
/////////////////////////////////////////////////////////////////
void CUpdaterComponentORACLE_VIRTUALBOX::install_post_interntal(bool success){
    if(success) CNotificationObserver::Instance()->Info(tr("Oracle VirtualBox has been installed."), DlgNotification::N_NO_ACTION);
}
