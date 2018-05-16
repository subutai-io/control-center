#include <map>

#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>

#include "updater/IUpdaterComponent.h"
#include "updater/HubComponentsUpdater.h"
#include "DlgNotification.h"
#include "SettingsManager.h"
#include "HubController.h"
#include "Commons.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "DownloadFileManager.h"

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
const QString IUpdaterComponent::CHROME = "Chrome";
const QString IUpdaterComponent::E2E = "e2e";
const QString IUpdaterComponent::VAGRANT_SUBUTAI = "vagrant_subutai";
const QString IUpdaterComponent::VAGRANT_VBGUEST = "vagrant_vbguest";

const QString &
IUpdaterComponent::component_id_to_user_view(const QString& id) {
  static std::map<QString, QString> dct = {
    {P2P, "P2P"},
    {TRAY, "ControlCenter"},
    {RH, "resource host"},
    {RHMANAGEMENT, "resource host management"},
    {X2GO, "X2Go-Client"},
    {VAGRANT, "Vagrant"},
    {ORACLE_VIRTUALBOX, "Oracle Virtualbox"},
    {CHROME, "Google Chrome"},
    {E2E, "Subutai E2E"},
    {VAGRANT_SUBUTAI, "Subutai plugin"},
    {VAGRANT_VBGUEST, "VirtualBox plugin"}
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
    {ORACLE_VIRTUALBOX, DlgNotification::N_ABOUT},
    {CHROME, DlgNotification::N_ABOUT},
    {VAGRANT_SUBUTAI, DlgNotification::N_ABOUT},
    {VAGRANT_VBGUEST, DlgNotification::N_ABOUT}
  };
  return dct.at(id);
}

//////////////////////////*X2GO-CLIENT*///////////////////////////////////////

CUpdaterComponentX2GO::CUpdaterComponentX2GO() {
  m_component_id = X2GO;
}
CUpdaterComponentX2GO::~CUpdaterComponentX2GO() {
}
QString CUpdaterComponentX2GO::download_x2go_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
bool CUpdaterComponentX2GO::update_available_internal(){
    QString version;
    CSystemCallWrapper::x2go_version(version);
    return version == "undefined";
}
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

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_X2GO);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentX2GO::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentX2GO::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentX2GO::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentX2GO::install_post_interntal(bool success){
    if(!success)
        CNotificationObserver::Instance()->Error(tr("X2Go-Client installation failed. It might be dependency problems. Install again, CC will solve them, or you can install it manualy."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("X2Go-Client has been installed. Congratulations!"), DlgNotification::N_NO_ACTION);
}

//////////////////////////*VAGRANT*///////////////////////////////////////

CUpdaterComponentVAGRANT::CUpdaterComponentVAGRANT() {
  m_component_id = VAGRANT;
}
CUpdaterComponentVAGRANT::~CUpdaterComponentVAGRANT() {
}
QString CUpdaterComponentVAGRANT::download_vagrant_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
bool CUpdaterComponentVAGRANT::update_available_internal(){
    QString version;
    CSystemCallWrapper::vagrant_version(version);
    return version == "undefined";
}
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

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_VAGRANT);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentVAGRANT::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentVAGRANT::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentVAGRANT::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentVAGRANT::install_post_interntal(bool success){if(!success)
        CNotificationObserver::Instance()->Error(tr("Vagrant installation failed. It might be dependency problems. Install again, CC will solve them, or you can install it manualy."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("Vagrant has been installed. Congratulations! Don't forget to install vagrant plugins."), DlgNotification::N_NO_ACTION);
}

///////////////////////////*VIRTUALBOX*///////////////////////////////////////

CUpdaterComponentORACLE_VIRTUALBOX::CUpdaterComponentORACLE_VIRTUALBOX() {
  m_component_id = ORACLE_VIRTUALBOX;
}
CUpdaterComponentORACLE_VIRTUALBOX::~CUpdaterComponentORACLE_VIRTUALBOX() {
}
QString CUpdaterComponentORACLE_VIRTUALBOX::download_oracle_virtualbox_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
bool CUpdaterComponentORACLE_VIRTUALBOX::update_available_internal(){
    QString version;
    CSystemCallWrapper::oracle_virtualbox_version(version);
    return version == "undefined";
}
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

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_VB);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation script."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentORACLE_VIRTUALBOX::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentORACLE_VIRTUALBOX::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentORACLE_VIRTUALBOX::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentORACLE_VIRTUALBOX::install_post_interntal(bool success){
    if(!success)
            CNotificationObserver::Instance()->Error(tr("Virtualbox installation failed. It might be dependency problems. Install again, CC will solve them, or you can install it manualy."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("Virtualbox has been installed. Congratulations!"), DlgNotification::N_NO_ACTION);
}

//////////////////////////*CHROME*///////////////////////////////////////

CUpdaterComponentCHROME::CUpdaterComponentCHROME() {
  m_component_id = CHROME;
}
CUpdaterComponentCHROME::~CUpdaterComponentCHROME() {
}
QString CUpdaterComponentCHROME::download_chrome_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
bool CUpdaterComponentCHROME::update_available_internal(){
    QString version;
    CSystemCallWrapper::chrome_version(version);
    return version == "undefined";
}
chue_t CUpdaterComponentCHROME::install_internal(){
    qDebug()
            << "Starting install chrome";
    QString file_name = chrome_kurjun_package_name();
    QString file_dir = download_chrome_path();
    QString str_downloaded_path = file_dir + "/" + file_name;

    std::vector<CGorjunFileInfo> fi = CRestWorker::Instance()->get_gorjun_file_info(file_name);
    if (fi.empty()) {
      qCritical("File %s isn't presented on kurjun", m_component_id.toStdString().c_str());
      return CHUE_NOT_ON_KURJUN;
    }
    std::vector<CGorjunFileInfo>::iterator item = fi.begin();

    CDownloadFileManager *dm = new CDownloadFileManager(item->id(),
                                                        str_downloaded_path,
                                                        item->size());

    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(file_dir, file_name, CC_CHROME);
    connect(dm, &CDownloadFileManager::download_progress_sig,
            [this](qint64 rec, qint64 total){update_progress_sl(rec, total+(total/5));});
    connect(dm, &CDownloadFileManager::finished,[silent_installer](){
        CNotificationObserver::Instance()->Info(tr("Running installation scripts."), DlgNotification::N_NO_ACTION);
        silent_installer->startWork();
    });
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentCHROME::install_finished_sl);
    connect(silent_installer, &SilentInstaller::outputReceived,
            dm, &CDownloadFileManager::deleteLater);
    dm->start_download();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentCHROME::update_internal(){
    update_progress_sl(100,100);
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentCHROME::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentCHROME::install_post_interntal(bool success){if(!success)
        CNotificationObserver::Instance()->Error(tr("Google Chrome installation failed. It might be dependency problems. Install again, CC will solve them, or you can install it manualy."), DlgNotification::N_NO_ACTION);
    else CNotificationObserver::Instance()->Info(tr("Google Chrome has been installed. Congratulations!"), DlgNotification::N_NO_ACTION);
}


//////////////////////////*E2E*///////////////////////////////////////

CUpdaterComponentE2E::CUpdaterComponentE2E() {
  m_component_id = E2E;
}
CUpdaterComponentE2E::~CUpdaterComponentE2E() {
}
QString CUpdaterComponentE2E::download_e2e_path(){
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    return (lst_temp.isEmpty() ? QApplication::applicationDirPath() : lst_temp[0]);
}
bool CUpdaterComponentE2E::update_available_internal(){
    QString version;
    CSystemCallWrapper::subutai_e2e_version(version);
    return version == "undefined";
}
chue_t CUpdaterComponentE2E::install_internal(){
    qDebug()
            << "Starting install subutai e2e";
    if(CSettingsManager::Instance().default_browser() == "Chrome"){
        QMessageBox *msg_box = new QMessageBox(QMessageBox::Information, QObject::tr("Attention!"),
                                               QObject::tr("Control Center will restart your Chrome. Make sure you saved all your work.\n"
                                                           "After installation make sure you will approve extension"),
                                               QMessageBox::Yes | QMessageBox::No);
        QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
        if (msg_box->exec() != QMessageBox::Yes) {
            install_finished_sl(false);
            return CHUE_SUCCESS;
        }
    }
    static QString empty_stings = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_stings, empty_stings, CC_E2E);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentE2E::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentE2E::update_internal(){
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentE2E::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentE2E::install_post_interntal(bool success){
    if(!success){
        CNotificationObserver::Instance()->Info(tr("Subutai E2E failed to install, we are sorry"), DlgNotification::N_NO_ACTION);
        return;
    }
    QMessageBox *msg_box = new QMessageBox(QMessageBox::Information, QObject::tr("Attention!"),
                                           QObject::tr("<br>Subutai E2E has been installed to your browser</br>"
                                                       "<br>If E2E does not appear, please approve installation from chrome://extensions.\n\t</br>"
                                                       "<br><a href='https://docs.subutai.io/Products/Bazaar/27_E2E_plugin.html'>"
                                                       "Learn more about Subutai E2E."
                                                       "</a></br>"),
                                           QMessageBox::Ok);
    msg_box->setTextFormat(Qt::RichText);
    QObject::connect(msg_box, &QMessageBox::finished, msg_box, &QMessageBox::deleteLater);
    if(msg_box->exec() == QMessageBox::Ok){
        CSystemCallWrapper::chrome_last_section();
    }
}


//////////////////////////*VAGRANT-SUBUTAI*///////////////////////////////////////

CUpdaterComponentVAGRANT_SUBUTAI::CUpdaterComponentVAGRANT_SUBUTAI() {
  m_component_id = VAGRANT_SUBUTAI;
}

CUpdaterComponentVAGRANT_SUBUTAI::~CUpdaterComponentVAGRANT_SUBUTAI() {
}

bool CUpdaterComponentVAGRANT_SUBUTAI::update_available_internal(){
    QString version;
    QString subutai_plugin = "vagrant-subutai";
    system_call_wrapper_error_t res = CSystemCallWrapper::vagrant_subutai_version(version);
    QString cloud_version =
        CRestWorker::Instance()->get_vagrant_plugin_cloud_version(subutai_plugin);
    if (version == "undefined") return true;
    if (res != SCWE_SUCCESS) return false;
    if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
    return cloud_version != version;
}
chue_t CUpdaterComponentVAGRANT_SUBUTAI::install_internal(){
    qDebug()
            << "Starting install vagrant subutai";
    update_progress_sl(50, 100); // imitation of progress bar :D, todo implement
    static QString empty_string = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_string, empty_string, CC_VAGRANT_SUBUTAI);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentVAGRANT_SUBUTAI::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentVAGRANT_SUBUTAI::update_internal(){
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentVAGRANT_SUBUTAI::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentVAGRANT_SUBUTAI::install_post_interntal(bool success){
    if(!success){
        CNotificationObserver::Instance()->Info(tr("Vagrant Subutai plugin failed to install, we are sorry"), DlgNotification::N_NO_ACTION);
    }
    else{
        CNotificationObserver::Instance()->Info(tr("Vagrant Subutai plugin has been installed successfully, congratulations!"), DlgNotification::N_NO_ACTION);
    }
}

//////////////////////////*VAGRANT-VBGUEST*///////////////////////////////////////

CUpdaterComponentVAGRANT_VBGUEST::CUpdaterComponentVAGRANT_VBGUEST() {
  m_component_id = VAGRANT_VBGUEST;
}

CUpdaterComponentVAGRANT_VBGUEST::~CUpdaterComponentVAGRANT_VBGUEST() {
}

bool CUpdaterComponentVAGRANT_VBGUEST::update_available_internal(){
    QString version;
    QString subutai_plugin = "vagrant-vbguest";
    system_call_wrapper_error_t res = CSystemCallWrapper::vagrant_vbguest_version(version);
    QString cloud_version =
        CRestWorker::Instance()->get_vagrant_plugin_cloud_version(subutai_plugin);
    if (version == "undefined") return true;
    if (res != SCWE_SUCCESS) return false;
    if (cloud_version == "undefined" || cloud_version.isEmpty()) return false;
    return cloud_version != version;
}
chue_t CUpdaterComponentVAGRANT_VBGUEST::install_internal(){
    qDebug()
            << "Starting install vagrant vbguest";
    update_progress_sl(50, 100);
    static QString empty_string = "";
    SilentInstaller *silent_installer = new SilentInstaller(this);
    silent_installer->init(empty_string, empty_string, CC_VAGRANT_VBGUEST);
    connect(silent_installer, &SilentInstaller::outputReceived,
            this, &CUpdaterComponentVAGRANT_VBGUEST::install_finished_sl);
    silent_installer->startWork();
    return CHUE_SUCCESS;
}
chue_t CUpdaterComponentVAGRANT_VBGUEST::update_internal(){
    update_finished_sl(true);
    return CHUE_SUCCESS;
}
void CUpdaterComponentVAGRANT_VBGUEST::update_post_action(bool success){
    UNUSED_ARG(success);
}
void CUpdaterComponentVAGRANT_VBGUEST::install_post_interntal(bool success){
    if(!success){
        CNotificationObserver::Instance()->Info(tr("Vagrant VirtualBox plugin failed to install, we are sorry"), DlgNotification::N_NO_ACTION);
    }
    else{
        CNotificationObserver::Instance()->Info(tr("Vagrant VirtualBox plugin has been installed successfully, congratulations!"), DlgNotification::N_NO_ACTION);
    }
}
