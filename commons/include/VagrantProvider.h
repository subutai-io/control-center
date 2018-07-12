#ifndef VAGRANTPROVIDER_H
#define VAGRANTPROVIDER_H

#include <QObject>
#include <QApplication>
#include <QApplication>

class VagrantProvider : QObject
{
Q_OBJECT
public:
  enum PROVIDERS {VIRTUALBOX = 0, PARALLELS, HYPERV, VMWARE_DESKTOP, LIBVIRT, PROVIDER_LAST = LIBVIRT};
  static VagrantProvider* Instance();
  static const QString& ProviderToStr(PROVIDERS p);
  static const QString& ProviderToVal(PROVIDERS p);
  static QString CurrentProvider();

private:
  VagrantProvider();
  virtual ~VagrantProvider();
};

#endif // VAGRANTPROVIDER_H
