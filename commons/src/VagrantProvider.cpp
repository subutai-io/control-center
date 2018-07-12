#include <VagrantProvider.h>
#include <SettingsManager.h>

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
