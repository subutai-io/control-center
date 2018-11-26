#include <VagrantProvider.h>
#include <SettingsManager.h>
#include <SystemCallWrapper.h>
#include <Commons.h>
#include <map>

// For Linux providers
std::vector<int> VagrantProvider::m_provider_linux = { VagrantProvider::VIRTUALBOX,
                                                       VagrantProvider::LIBVIRT,
                                                       VagrantProvider::VMWARE_DESKTOP
                                                     };
// For Darwin providers
std::vector<int> VagrantProvider::m_provider_darwin = { VagrantProvider::VIRTUALBOX,
                                                        VagrantProvider::PARALLELS,
                                                        VagrantProvider::VMWARE_DESKTOP
                                                      };
// For Windows providers
std::vector<int> VagrantProvider::m_provider_win = { VagrantProvider::VIRTUALBOX,
                                                     VagrantProvider::HYPERV,
                                                     VagrantProvider::VMWARE_DESKTOP
                                                   };

/////////////////////////////////////////////////////////////////////////////////////////////////////
VagrantProvider::VagrantProvider() {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
VagrantProvider::~VagrantProvider() {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
VagrantProvider* VagrantProvider::Instance() {
  static VagrantProvider instance;

  return &instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const QString& VagrantProvider::ProviderToStr(PROVIDERS p) {
  static QString p_str[] = {tr("Virtualbox"), tr("VMware"),
                            tr("Hyper-V"), tr("Parallels"),
                            tr("Libvirt"),
                            };
  return p_str[p];
}

const QString& VagrantProvider::ProviderToName(PROVIDERS p) {
  static std::map<PROVIDERS, QString> p_name = {{LIBVIRT, "vagrant-libvirt"}, {VMWARE_DESKTOP, "vagrant-vmware-desktop"},
                                           {PARALLELS, "vagrant-parallels"}};
  return p_name[p];
}

const QString& VagrantProvider::CurrentOpenFileTitle() {
  static std::map<PROVIDERS, QString> p_title = {{VIRTUALBOX, tr("VirtualBox Command")},
                                                 {VMWARE_DESKTOP, tr("VMware Command")},
                                                 {PARALLELS, tr("Parallels Command")},
                                                 {LIBVIRT, tr("KVM command")}
                                                };
  return p_title[CurrentProvider()];
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
const QString& VagrantProvider::ProviderToVal(PROVIDERS p) {
  static QString p_val[] = {"virtualbox", "vmware_desktop",
                            "hyperv", "parallels",
                            "libvirt",
                            };
  return p_val[p];
}

QString VagrantProvider::CurrentVal() {
  QString provider = VagrantProvider::ProviderToVal(VagrantProvider::CurrentProvider());

  return provider;
}

QString VagrantProvider::CurrentName() {
  QString provider = VagrantProvider::ProviderToName(VagrantProvider::CurrentProvider());

  return provider;
}

VagrantProvider::PROVIDERS VagrantProvider::CurrentProvider() {
  VagrantProvider::PROVIDERS provider = VagrantProvider::VIRTUALBOX;
  VagrantProvider::PROVIDERS current = static_cast<VagrantProvider::PROVIDERS>(
        CSettingsManager::Instance().vagrant_provider());
  std::vector<int> providers = VagrantProvider::List(); // list of providers by os

  if (VagrantProvider::PROVIDER_LAST >= current &&
      std::find(providers.begin(), providers.end(), current) != providers.end()) {
    provider = current;
  }

  return provider;
}

// check which provider will use port of peer
// CC uses port of peer(hyperv, libvirt)
bool VagrantProvider::UseIp() {
  PROVIDERS provider = CurrentProvider();

  if (provider == HYPERV || provider == LIBVIRT)
    return true;

  return false;
}

QString VagrantProvider::CurrentStr() {
  QString provider = VagrantProvider::ProviderToStr(VagrantProvider::CurrentProvider());

  return provider;
}

QDir VagrantProvider::BasePeerDirVirtualbox() {
  QDir base_peer_dir_virt;
  QStringList stdDirList =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QStringList::iterator stdDir = stdDirList.begin();
  if (stdDir == stdDirList.end())
    base_peer_dir_virt.setCurrent("/");
  else
    base_peer_dir_virt.setCurrent(*stdDir);

  base_peer_dir_virt.mkdir("Subutai-peers");
  base_peer_dir_virt.cd("Subutai-peers");

  return base_peer_dir_virt;
}

QDir VagrantProvider::BasePeerDirHyperv() {
  QString base_peer_path = CSettingsManager::Instance().hyperv_vm_storage();

  if (!base_peer_path.contains("Subutai-peers") &&
      !QDir().exists(base_peer_path + QDir::separator() + "Subutai-peers")) {
    base_peer_path = base_peer_path + QDir::separator() + "Subutai-peers";
    QDir().mkdir(base_peer_path);
  }

  // 2. create "peer" folder.
  QString empty;
  QDir base_peer_dir_hyperv(base_peer_path);
  base_peer_dir_hyperv.cd("Subutai-peers");

  return base_peer_dir_hyperv;
}

QDir VagrantProvider::BasePeerDirVMware() {
  QString base_peer_path = CSettingsManager::Instance().vmware_vm_storage();

  if (!base_peer_path.contains("Subutai-peers") &&
      !QDir().exists(base_peer_path + QDir::separator() + "Subutai-peers")) {
    base_peer_path = base_peer_path + QDir::separator() + "Subutai-peers";
    QDir().mkdir(base_peer_path);
  }

  // 2. create "peer" folder.
  QString empty;
  QDir base_peer_dir_vmware(base_peer_path);
  base_peer_dir_vmware.cd("Subutai-peers");

  return base_peer_dir_vmware;
}

QDir VagrantProvider::BasePeerDirParallels() {
  QString base_peer_path = CSettingsManager::Instance().parallels_vm_storage();

  if (!base_peer_path.contains("Subutai-peers") &&
      !QDir().exists(base_peer_path + QDir::separator() + "Subutai-peers")) {
    base_peer_path = base_peer_path + QDir::separator() + "Subutai-peers";
    QDir().mkdir(base_peer_path);
  }

  // 2. create "peer" folder.
  QString empty;
  QDir base_peer_dir_parallels(base_peer_path);
  base_peer_dir_parallels.cd("Subutai-peers");

  return base_peer_dir_parallels;
}

QDir VagrantProvider::BasePeerDir() {
  switch (CurrentProvider()) {
  case VIRTUALBOX:
    return BasePeerDirVirtualbox();
  case VMWARE_DESKTOP:
    return BasePeerDirVMware();
  case HYPERV:
    return BasePeerDirHyperv();
  case PARALLELS:
    return BasePeerDirParallels();
  default:
    return BasePeerDirVirtualbox();
  }
}

QString VagrantProvider::VmStorage() {
  switch (CurrentProvider()) {
  case HYPERV:
    return CSettingsManager::Instance().hyperv_vm_storage();
  case VMWARE_DESKTOP:
    return CSettingsManager::Instance().vmware_vm_storage();
  case VIRTUALBOX:
    return CSystemCallWrapper::get_virtualbox_vm_storage();
  case PARALLELS:
    return CSettingsManager::Instance().parallels_vm_storage();
  default:
    return CCommons::HomePath() + QDir::separator() + CCommons::PEER_PATH;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
template <>
std::vector <int> VagrantProvider::list_by_os<Os2Type <OS_MAC> >() {
  return m_provider_darwin;
}

template <>
std::vector <int> VagrantProvider::list_by_os<Os2Type <OS_LINUX> >() {
  return m_provider_linux;
}

template <>
std::vector <int> VagrantProvider::list_by_os<Os2Type <OS_WIN> >() {
  return m_provider_win;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<int> VagrantProvider::List() {
  return list_by_os<Os2Type<CURRENT_OS> >();
}
