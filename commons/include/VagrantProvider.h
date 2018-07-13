#ifndef VAGRANTPROVIDER_H
#define VAGRANTPROVIDER_H

#include <QObject>
#include <QApplication>
#include <QApplication>
#include "OsBranchConsts.h"
#include <vector>

class VagrantProvider : QObject
{
Q_OBJECT
public:
  enum PROVIDERS {VIRTUALBOX = 0, PARALLELS, HYPERV,
                  VMWARE_DESKTOP, LIBVIRT,
                  PROVIDER_LAST = LIBVIRT};

  static std::vector<int> m_provider_linux,
                          m_provider_darwin,
                          m_provider_win;

  static VagrantProvider* Instance();
  static const QString& ProviderToStr(PROVIDERS p);
  static const QString& ProviderToVal(PROVIDERS p);
  static QString CurrentProvider();
  static QString CurrentName();
  static std::vector<int> List();

  template <class OS> static
  std::vector<int> list_by_os();

private:
  VagrantProvider();
  virtual ~VagrantProvider();
};

#endif // VAGRANTPROVIDER_H
