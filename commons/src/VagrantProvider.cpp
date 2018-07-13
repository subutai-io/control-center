#include <VagrantProvider.h>
#include <SettingsManager.h>

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
  static QString p_str[] = {tr("Virtualbox"), tr("Parallels"), tr("Hyper-V"), tr("VMware"), tr("Libvirt")};
  return p_str[p];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
const QString& VagrantProvider::ProviderToVal(PROVIDERS p) {
  static QString p_val[] = {"virtualbox", "parallels", "hyperv", "vmware_desktop", "libvirt"};
  return p_val[p];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
QString VagrantProvider::CurrentProvider() {
  QString provider = VagrantProvider::ProviderToVal(VagrantProvider::VIRTUALBOX);

  if (VagrantProvider::PROVIDER_LAST >= CSettingsManager::Instance().vagrant_provider()) {
    provider = VagrantProvider::ProviderToVal(
             (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider());
  }

  return provider;
}

QString VagrantProvider::CurrentName() {
  QString provider = VagrantProvider::ProviderToVal(VagrantProvider::VIRTUALBOX);

  if (VagrantProvider::PROVIDER_LAST >= CSettingsManager::Instance().vagrant_provider()) {
    provider = VagrantProvider::ProviderToStr(
             (VagrantProvider::PROVIDERS)CSettingsManager::Instance().vagrant_provider());
  }

  return provider;
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
