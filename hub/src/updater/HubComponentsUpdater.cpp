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
#include "updater/UpdaterComponentVagrant.h"
#include "updater/UpdaterComponentX2go.h"
#include "updater/UpdaterComponentVirtualbox.h"
#include "updater/UpdaterComponentChrome.h"
#include "updater/UpdaterComponentE2E.h"
#include "updater/UpdaterComponentSubutaiBox.h"
#include "updater/UpdaterComponentVagrantSubutai.h"
#include "updater/UpdaterComponentVagrantVBguest.h"
#include "updater/UpdaterComponentVagrantParallels.h"
#include "updater/UpdaterComponentVagrantLibvirt.h"
#include "updater/UpdaterComponentVagrantVmware.h"
#include "updater/UpdaterComponentVMware.h"
#include "updater/UpdaterComponentVagrantVMwareUtility.h"
#include "updater/UpdaterComponentFirefox.h"
#include "updater/UpdaterComponentXQuartz.h"
#include "updater/UpdaterComponentHyperv.h"
#include "updater/UpdaterComponentKvm.h"
#include "updater/IUpdaterComponent.h"
#include "updater/UpdaterComponentParallels.h"


using namespace update_system;

CHubComponentsUpdater::CHubComponentsUpdater() {
  IUpdaterComponent *uc_tray, *uc_p2p, *uc_x2go, *uc_firefox,
          *uc_vagrant, *uc_oracle_virtualbox, *uc_chrome, *uc_e2e,
          *uc_vagrant_subutai, *uc_vagrant_vbguest, *uc_vagrant_parallels,
          *uc_vagrant_vmware, *uc_vagrant_libvirt, *uc_subutai_box,
          *uc_hypervisor_vmware, *uc_vagrant_vmware_utility, *uc_xquartz,
          *uc_hyperv, *uc_parallels, *uc_kvm;

  uc_tray = new CUpdaterComponentTray;
  uc_p2p  = new CUpdaterComponentP2P;
  uc_x2go = new CUpdaterComponentX2GO;
  uc_vagrant = new CUpdaterComponentVAGRANT;
  uc_oracle_virtualbox = new CUpdaterComponentVIRTUALBOX;
  uc_chrome = new CUpdaterComponentCHROME;
  uc_firefox = new CUpdaterComponentFIREFOX;
  uc_e2e = new CUpdaterComponentE2E;
  uc_vagrant_subutai = new CUpdaterComponentVAGRANT_SUBUTAI;
  uc_vagrant_vbguest = new CUpdaterComponentVAGRANT_VBGUEST;
  uc_subutai_box = new CUpdaterComponentSUBUTAI_BOX;
  uc_vagrant_parallels = new CUpdaterComponentVAGRANT_PARALLELS;
  uc_vagrant_libvirt = new CUpdaterComponentVAGRANT_LIBVIRT;
  uc_vagrant_vmware = new CUpdaterComponentVAGRANT_VMWARE;
  uc_hypervisor_vmware = new CUpdaterComponentVMware;
  uc_vagrant_vmware_utility = new CUpdaterComponentVagrantVMwareUtility;
  uc_xquartz = new CUpdaterComponentXQuartz;
  uc_hyperv = new CUpdaterComponentHyperv;
  uc_kvm = new CUpdaterComponentKvm;
  uc_parallels = new CUpdaterComponentParallels;

  IUpdaterComponent* ucs[] = {uc_tray, uc_p2p,
                              uc_x2go, uc_vagrant, uc_oracle_virtualbox,
                              uc_chrome, uc_firefox, uc_e2e, uc_vagrant_subutai,
                              uc_vagrant_vbguest, uc_subutai_box, uc_vagrant_parallels,
                              uc_vagrant_libvirt, uc_vagrant_vmware, uc_hypervisor_vmware,
                              uc_vagrant_vmware_utility, uc_xquartz, uc_hyperv, uc_parallels,
                              uc_kvm,
                              nullptr};

  m_dct_components[IUpdaterComponent::TRAY] = CUpdaterComponentItem(uc_tray);
  m_dct_components[IUpdaterComponent::P2P]  = CUpdaterComponentItem(uc_p2p);
  m_dct_components[IUpdaterComponent::X2GO] = CUpdaterComponentItem(uc_x2go);
  m_dct_components[IUpdaterComponent::VAGRANT] = CUpdaterComponentItem(uc_vagrant);
  m_dct_components[IUpdaterComponent::ORACLE_VIRTUALBOX] = CUpdaterComponentItem(uc_oracle_virtualbox);
  m_dct_components[IUpdaterComponent::CHROME] = CUpdaterComponentItem(uc_chrome);
  m_dct_components[IUpdaterComponent::FIREFOX] = CUpdaterComponentItem(uc_firefox);
  m_dct_components[IUpdaterComponent::E2E] = CUpdaterComponentItem(uc_e2e);
  m_dct_components[IUpdaterComponent::VAGRANT_SUBUTAI] = CUpdaterComponentItem(uc_vagrant_subutai);
  m_dct_components[IUpdaterComponent::VAGRANT_VBGUEST] = CUpdaterComponentItem(uc_vagrant_vbguest);
  m_dct_components[IUpdaterComponent::SUBUTAI_BOX] = CUpdaterComponentItem(uc_subutai_box);
  m_dct_components[IUpdaterComponent::VAGRANT_PARALLELS] = CUpdaterComponentItem(uc_vagrant_parallels);
  m_dct_components[IUpdaterComponent::VAGRANT_LIBVIRT] = CUpdaterComponentItem(uc_vagrant_libvirt);
  m_dct_components[IUpdaterComponent::VAGRANT_VMWARE_DESKTOP] = CUpdaterComponentItem(uc_vagrant_vmware);
  m_dct_components[IUpdaterComponent::VMWARE] = CUpdaterComponentItem(uc_hypervisor_vmware);
  m_dct_components[IUpdaterComponent::VMWARE_UTILITY] = CUpdaterComponentItem(uc_vagrant_vmware_utility);
  m_dct_components[IUpdaterComponent::XQUARTZ] = CUpdaterComponentItem(uc_xquartz);
  m_dct_components[IUpdaterComponent::HYPERV] = CUpdaterComponentItem(uc_hyperv);
  m_dct_components[IUpdaterComponent::KVM] = CUpdaterComponentItem(uc_kvm);
  m_dct_components[IUpdaterComponent::PARALLELS] = CUpdaterComponentItem(uc_parallels);

  for(int i = 0; ucs[i] ;++i) {
    connect(&m_dct_components[ucs[i]->component_id()], &CUpdaterComponentItem::timer_timeout,
        this, &CHubComponentsUpdater::update_component_timer_timeout);
    connect(ucs[i], &IUpdaterComponent::update_progress,
        this, &CHubComponentsUpdater::update_component_progress_sl);
    connect(ucs[i], &IUpdaterComponent::update_finished,
        this, &CHubComponentsUpdater::update_component_finished_sl);
    connect(ucs[i], &IUpdaterComponent::install_finished,
            this, &CHubComponentsUpdater::install_component_finished_sl);
    connect(ucs[i], &IUpdaterComponent::uninstall_finished,
            this, &CHubComponentsUpdater::uninstall_component_finished_sl);
  }
  ///
  set_p2p_update_freq();
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
    qCritical(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }

  m_dct_components[component_id].timer_stop();
  if (freq == CSettingsManager::UF_NEVER)
    return;

  m_dct_components[component_id].set_timer_interval(
        CSettingsManager::update_freq_to_sec(freq) * 1000);
  m_dct_components[component_id].timer_start();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_component_autoupdate(const QString &component_id,
                                                bool autoupdate) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    qCritical(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }
  m_dct_components[component_id].autoupdate = autoupdate;
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::update_component_timer_timeout(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    qCritical(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }
  if (component_id == IUpdaterComponent::P2P) {
    QString p2p_old_version;
    CSystemCallWrapper::p2p_version(p2p_old_version);
    if (p2p_old_version == "undefined"){
      return;
    }
  }
  m_dct_components[component_id].timer_stop();
  if (m_dct_components[component_id].Component()->update_available()) {
    if (m_dct_components[component_id].autoupdate) {
      CNotificationObserver::Instance()->Info(
            tr("%1 updating started.").arg(IUpdaterComponent::component_id_to_user_view(component_id)),  DlgNotification::N_NO_ACTION);
      m_dct_components[component_id].Component()->update();
    } else {
      QString update_message = tr("New version of %1 is available! "
                                  "You can read release notes <a href=\"%2\">here</a>");
      CNotificationObserver::Instance()->Info(
            update_message
                  .arg(IUpdaterComponent::component_id_to_user_view(component_id),
                       IUpdaterComponent::component_id_changelog(component_id)),
                  IUpdaterComponent::component_id_to_notification_action(component_id));
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
CHubComponentsUpdater::set_tray_update_freq() {
  set_update_freq(IUpdaterComponent::TRAY, CSettingsManager::Instance().tray_update_freq());
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::set_p2p_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::P2P,
                           CSettingsManager::Instance().p2p_autoupdate());
}
////////////////////////////////////////////////////////////////////////////
void
CHubComponentsUpdater::set_tray_autoupdate() {
  set_component_autoupdate(IUpdaterComponent::TRAY,
                           CSettingsManager::Instance().tray_autoupdate());
}
////////////////////////////////////////////////////////////////////////////

bool
CHubComponentsUpdater::is_update_available(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    qCritical(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return false;
  }
  return m_dct_components[component_id].Component()->update_available();
}
////////////////////////////////////////////////////////////////////////////

void
CHubComponentsUpdater::force_update(const QString &component_id) {
  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    qCritical(
          "can't find component updater in map with id = %s", component_id.toStdString().c_str());
    return;
  }
  m_dct_components[component_id].Component()->update();
}

void CHubComponentsUpdater::force_update_p2p() {
  force_update(IUpdaterComponent::P2P);
}

void CHubComponentsUpdater::force_update_tray() {
  force_update(IUpdaterComponent::TRAY);
}


////////////////////////////////////////////////////////////////////////////
/// \brief CHubComponentsUpdater::install
/// \param component_id
///
void CHubComponentsUpdater::install(const QString &component_id) {
    qDebug("Install component: %s started.",
           component_id.toStdString().c_str());

    if (m_dct_components.find(component_id) == m_dct_components.end()) {
      qCritical("Can't find component updater in map with id = %s",
                component_id.toStdString().c_str());
      return;
    }

    emit install_component_started(IUpdaterComponent::component_id_to_user_view(component_id));
    m_dct_components[component_id].Component()->install();
}

////////////////////////////////////////////////////////////////////////////
/// \brief CHubComponentsUpdater::uninstall
/// \param component_id
///
void CHubComponentsUpdater::uninstall(const QString &component_id) {
  qDebug("Uninstall component: %s started.",
         component_id.toStdString().c_str());

  if (m_dct_components.find(component_id) == m_dct_components.end()) {
    qCritical("Can't find component by id=%s in updater map",
              component_id.toStdString().c_str());
    return;
  }

  emit uninstall_component_started(IUpdaterComponent::component_id_to_user_view(component_id));
  m_dct_components[component_id].Component()->uninstall();
}

void CHubComponentsUpdater::install_p2p(){
    install(IUpdaterComponent::P2P);
}

void CHubComponentsUpdater::install_x2go(){
    install(IUpdaterComponent::X2GO);
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
void CHubComponentsUpdater::install_component_finished_sl(const QString &file_id, bool replaced,
                                                          const QString& version) {
    emit installing_finished(file_id, replaced, version);
}

void CHubComponentsUpdater::uninstall_component_finished_sl(const QString &component_id, bool success,
                                                            const QString& version) {
  emit uninstalling_finished(component_id, success, version);
}
/////////////////////////////////////////////////////////////////////////////
QString CHubComponentsUpdater::component_name(const QString &component_id) {
    if(m_dct_components.find(component_id) == m_dct_components.end())
        return "";
    return m_dct_components[component_id].Component()->component_id_to_user_view(component_id);
}

/////////////////////////////////////////////////////////////////////////////
bool CHubComponentsUpdater::is_in_progress(const QString &component_id){
    if(m_dct_components.find(component_id) == m_dct_components.end())
        return true;
    else return m_dct_components[component_id].Component()->is_in_progress();
}

const std::pair <quint64, quint64>& CHubComponentsUpdater::get_last_pb_value(const QString &component_id) {
  static std::pair <quint64, quint64> zero(0, 100);
  if (m_dct_components.find(component_id) == m_dct_components.end() || !is_in_progress(component_id)) {
    return zero;
  } else {
    return m_dct_components[component_id].Component()->get_last_pb_value();
  }
}

///////* class installs cc components in silent mode *///////////
void SilentInstaller::init(const QString &dir, const QString &file_name, cc_component type) {
    m_dir = dir;
    m_file_name = file_name;
    m_type = type;
}

void SilentInstaller::startWork(){
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentInstaller::silentInstallation);
    connect(this, &SilentInstaller::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentInstaller::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
}

void SilentInstaller::silentInstallation(){
    QFutureWatcher<system_call_wrapper_install_t> *watcher
        = new QFutureWatcher<system_call_wrapper_install_t>(this);
    QFuture<system_call_wrapper_install_t>  res;

    static QString subutai_plugin_name = "vagrant-subutai";
    static QString vbguest_plugin_name = "vagrant-vbguest";
    static QString parallels_provider = "vagrant-parallels";
    static QString vmware_provider = "vagrant-vmware-desktop";
    static QString libvirt_provider = "vagrant-libvirt";
    static QString command = "install";

    switch (m_type) {
    case CC_P2P:
        res = QtConcurrent::run(CSystemCallWrapper::install_p2p, m_dir, m_file_name);
        break;
    case CC_CHROME:
        res = QtConcurrent::run(CSystemCallWrapper::install_chrome, m_dir, m_file_name);
        break;
    case CC_FIREFOX:
        res = QtConcurrent::run(CSystemCallWrapper::install_firefox, m_dir, m_file_name);
        break;
    case CC_VAGRANT:
        res = QtConcurrent::run(CSystemCallWrapper::install_vagrant, m_dir, m_file_name);
        break;
    case CC_VB:
        res = QtConcurrent::run(CSystemCallWrapper::install_oracle_virtualbox, m_dir, m_file_name);
        break;
    case CC_VMWARE:
        res = QtConcurrent::run(CSystemCallWrapper::install_vmware, m_dir, m_file_name);
        break;
    case CC_VMWARE_UTILITY:
        res = QtConcurrent::run(CSystemCallWrapper::install_vmware_utility, m_dir, m_file_name);
        break;
    case CC_X2GO:
        res = QtConcurrent::run(CSystemCallWrapper::install_x2go, m_dir, m_file_name);
        break;
    case CC_E2E:
        res = QtConcurrent::run(CSystemCallWrapper::install_e2e, m_dir, m_file_name);
        break;
    case CC_VAGRANT_SUBUTAI:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, subutai_plugin_name, command);
        break;
    case CC_VAGRANT_VBGUEST:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vbguest_plugin_name, command);
        break;
    case CC_SUBUTAI_BOX:
        res = QtConcurrent::run(CSystemCallWrapper::install_subutai_box, m_dir, m_file_name);
        break;
    case CC_VAGRANT_LIBVIRT:
        res = QtConcurrent::run(CSystemCallWrapper::install_vagrant_libvirt);
        break;
    case CC_VAGRANT_VMWARE_DESKTOP:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vmware_provider, command);
        break;
    case CC_VAGRANT_PARALLELS:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, parallels_provider, command);
        break;
    case CC_XQUARTZ:
        res = QtConcurrent::run(CSystemCallWrapper::install_xquartz, m_dir, m_file_name);
        break;
    case CC_HYPERV:
        res = QtConcurrent::run(CSystemCallWrapper::install_hyperv);
        break;
    case CC_KVM:
        res = QtConcurrent::run(CSystemCallWrapper::install_kvm);
        break;
    case CC_PARALLELS:
        res = QtConcurrent::run(CSystemCallWrapper::install_parallels, m_dir, m_file_name);
        break;
    default:
        break;
    }
    connect(watcher, &QFutureWatcher<system_call_wrapper_install_t>::finished, [this, res]() {
      qDebug() << "SilentInstaller output received version: "
               << res.result().version;
      emit this->outputReceived(res.result().res == SCWE_SUCCESS, res.result().version);
    });
    watcher->setFuture(res);
}

///////* class uninstalls cc components in silent mode *///////////
void SilentUninstaller::init(const QString &dir, const QString &file_name, cc_component type) {
  m_dir = dir;
  m_file_name = file_name;
  m_type = type;
}

void SilentUninstaller::startWork() {
  QThread* thread = new QThread();
  connect(thread, &QThread::started,
          this, &SilentUninstaller::silentUninstallation);
  connect(this, &SilentUninstaller::outputReceived,
          thread, &QThread::quit);
  connect(thread, &QThread::finished,
          this, &SilentUninstaller::deleteLater);
  connect(thread, &QThread::finished,
          thread, &QThread::deleteLater);
  this->moveToThread(thread);
  thread->start();
}

void SilentUninstaller::silentUninstallation() {
  QFutureWatcher<system_call_wrapper_install_t> *watcher
      = new QFutureWatcher<system_call_wrapper_install_t>(this);
  QFuture<system_call_wrapper_install_t>  res;

  static QString subutai_plugin_name = "vagrant-subutai";
  static QString vbguest_plugin_name = "vagrant-vbguest";
  static QString parallels_provider = "vagrant-parallels";
  static QString vmware_provider = "vagrant-vmware-desktop";
  static QString libvirt_provider = "vagrant-libvirt";

  static QString command = "uninstall";

  switch (m_type) {
  case CC_VAGRANT_SUBUTAI:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, subutai_plugin_name, command);
    break;
  case CC_VAGRANT_VBGUEST:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vbguest_plugin_name, command);
    break;
  case CC_SUBUTAI_BOX:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_box_remove, subutai_box_name(), VagrantProvider::Instance()->CurrentVal());
    break;
  case CC_VAGRANT_LIBVIRT:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, libvirt_provider, command);
    break;
  case CC_VAGRANT_VMWARE_DESKTOP:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vmware_provider, command);
    break;
  case CC_VAGRANT_PARALLELS:
    res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, parallels_provider, command);
    break;
  case CC_X2GO:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_x2go);
    break;
  case CC_VAGRANT:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_vagrant, m_dir, m_file_name);
    break;
  case CC_P2P:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_p2p, m_dir, m_file_name);
    break;
  case CC_E2E:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_e2e);
    break;
  case CC_CHROME:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_chrome, m_dir, m_file_name);
    break;
  case CC_FIREFOX:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_firefox, m_dir, m_file_name);
    break;
  case CC_VB:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_oracle_virtualbox, m_dir, m_file_name);
    break;
  case CC_VMWARE:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_vmware, m_dir, m_file_name);
    break;
  case CC_VMWARE_UTILITY:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_vmware_utility, m_dir, m_file_name);
    break;
  case CC_XQUARTZ:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_xquartz);
    break;
  case CC_HYPERV:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_hyperv);
    break;
  case CC_KVM:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_kvm);
    break;
  case CC_PARALLELS:
    res = QtConcurrent::run(CSystemCallWrapper::uninstall_parallels, m_dir, m_file_name);
    break;
  default:
    break;
  }

  connect(watcher, &QFutureWatcher<system_call_wrapper_install_t>::finished, [this, res]() {
    emit this->outputReceived(res.result().res == SCWE_SUCCESS, res.result().version);
  });
  watcher->setFuture(res);
}

///////* class updates cc components in silent mode *///////////
void SilentUpdater::init(const QString &dir, const QString &file_name, cc_component type) {
    m_dir = dir;
    m_file_name = file_name;
    m_type = type;
}

void SilentUpdater::startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentUpdater::silentUpdate);
    connect(this, &SilentUpdater::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentUpdater::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
}

void SilentUpdater::silentUpdate() {
    QFutureWatcher<system_call_wrapper_install_t> *watcher
        = new QFutureWatcher<system_call_wrapper_install_t>(this);
    QFuture<system_call_wrapper_install_t>  res;

    static QString subutai_plugin_name = "vagrant-subutai";
    static QString vbguest_plugin_name = "vagrant-vbguest";
    static QString parallels_provider = "vagrant-parallels";
    static QString vmware_provider = "vagrant-vmware-desktop";
    static QString libvirt_provider = "vagrant-libvirt";
    static QString command = "update";

    switch (m_type) {
    case CC_VAGRANT_SUBUTAI:
      res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, subutai_plugin_name, command);
      break;
    case CC_VAGRANT_VBGUEST:
      res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vbguest_plugin_name, command);
      break;
    case CC_VAGRANT_LIBVIRT:
      res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, libvirt_provider, command);
      break;
    case CC_VAGRANT_VMWARE_DESKTOP:
      res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, vmware_provider, command);
      break;
    case CC_VAGRANT_PARALLELS:
      res = QtConcurrent::run(CSystemCallWrapper::vagrant_plugin, parallels_provider, command);
      break;
    case CC_P2P:
      res = QtConcurrent::run(CSystemCallWrapper::update_p2p, m_dir, m_file_name);
      break;
    default:
      break;
    }

    connect(watcher, &QFutureWatcher<system_call_wrapper_install_t>::finished, [this, res](){
      emit this->outputReceived(res.result().res == SCWE_SUCCESS);
    });
    watcher->setFuture(res);
}
