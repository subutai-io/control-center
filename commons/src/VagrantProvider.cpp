#include <VagrantProvider.h>
#include <SettingsManager.h>
#include <map>

// For Linux providers
std::vector<int> VagrantProvider::m_provider_linux = { VagrantProvider::VIRTUALBOX,
                                                     //  VagrantProvider::LIBVIRT,
                                                       VagrantProvider::VMWARE_DESKTOP
                                                     };
// For Darwin providers
std::vector<int> VagrantProvider::m_provider_darwin = { VagrantProvider::VIRTUALBOX,
                                                        VagrantProvider::PARALLELS,
                                                        VagrantProvider::VMWARE_DESKTOP
                                                      };
// For Windows providers
std::vector<int> VagrantProvider::m_provider_win = { VagrantProvider::VIRTUALBOX,
                                                    // VagrantProvider::HYPERV,
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
                            tr("Parallels"), tr("Hyper-V"),
                            tr("Libvirt")};

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
                                                 {PARALLELS, tr("Parallels Command")}
                                                };
  return p_title[CurrentProvider()];
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
const QString& VagrantProvider::ProviderToVal(PROVIDERS p) {
  static QString p_val[] = {"virtualbox", "vmware_desktop",
                            "parallels", "hyperv",
                            "libvirt"};
  return p_val[p];
}

QString VagrantProvider::CurrentVal() {
  QString provider = VagrantProvider::ProviderToVal(VagrantProvider::VIRTUALBOX);

  if (VagrantProvider::PROVIDER_LAST >= CSettingsManager::Instance().vagrant_provider()) {
    provider = VagrantProvider::ProviderToVal(
             (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider());
  }

  return provider;
}

QString VagrantProvider::CurrentName() {
  QString provider = VagrantProvider::ProviderToName(VagrantProvider::VIRTUALBOX);

  if (VagrantProvider::PROVIDER_LAST >= CSettingsManager::Instance().vagrant_provider()) {
    provider = VagrantProvider::ProviderToName(
             (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider());
  }

  return provider;
}

VagrantProvider::PROVIDERS VagrantProvider::CurrentProvider() {
  VagrantProvider::PROVIDERS provider = VagrantProvider::VIRTUALBOX;

  if (VagrantProvider::PROVIDER_LAST >= (int)CSettingsManager::Instance().vagrant_provider()) {
    provider = (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider();
  }

  return provider;
}

QString VagrantProvider::CurrentStr() {
  QString provider = VagrantProvider::ProviderToStr(VagrantProvider::VIRTUALBOX);

  if (VagrantProvider::PROVIDER_LAST >= (int)CSettingsManager::Instance().vagrant_provider()) {
    provider = VagrantProvider::ProviderToStr(
             (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider());
  }

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

QDir VagrantProvider::BasePeerDir() {
  switch (CurrentProvider()) {
  case VIRTUALBOX:
    return BasePeerDirVirtualbox();
  case VMWARE_DESKTOP:
    return BasePeerDirVMware();
  default:
    return BasePeerDirVirtualbox();
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
