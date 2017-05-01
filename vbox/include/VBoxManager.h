#ifndef VBOXMANAGER_H
#define VBOXMANAGER_H

#include <QObject>
#include <QTimer>
#include "VirtualMachine.h"
#include <stdint.h>

class CVboxManager : public QObject {
  Q_OBJECT
private:

  std::map<QString, CVirtualMachine*> m_dct_machines;
  QString m_version;
  QTimer* m_refresh_timer;

  void update_machine_state(const QString& vm_id);

  CVboxManager(QObject *parent = nullptr);
  virtual ~CVboxManager();
  int32_t init_machines();

public:

  static CVboxManager* Instance() {
    static CVboxManager inst;
    return &inst;
  }

  const std::map<QString, CVirtualMachine*>& dct_machines() const {
    return m_dct_machines;
  }

  const QString& version() const {return m_version;}

  CVirtualMachine* vm_by_id(const QString& vm_id);
  int32_t launch_vm(const QString& vm_id);
  int32_t pause(const QString& vm_id);
  int32_t resume(const QString& vm_id);
  int32_t poweroff(const QString& vm_id);
  int32_t add(const QString& vm_id);
  int32_t remove(const QString& vm_id);

private slots:
  void refresh_timer_timeout();
public slots:
  void start_work();

signals:
  void vm_add(const QString& vm_id);
  void vm_remove(const QString& vm_id);
  void vm_state_changed(const QString& vm_id);

  void initialized();
  void finished();
};

#endif // VBOXMANAGER_H
