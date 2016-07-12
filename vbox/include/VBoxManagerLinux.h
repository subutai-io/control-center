#ifndef VBOX_MANAGER_LINUX_H
#define VBOX_MANAGER_LINUX_H

#include <stdint.h>

#include "VBoxCommons.h"
#include "VirtualBoxHdr.h"
#include "IVBoxManager.h"

class CVBoxManagerLinux : public IVBoxManager
{
private:
  friend class CVBoxManagerSingleton;

  CVBoxManagerLinux();
  virtual ~CVBoxManagerLinux();

  nsCOMPtr<nsIServiceManager> m_service_manager;
  nsCOMPtr<IVirtualBox> m_virtual_box;
  nsCOMPtr<nsIComponentManager> m_component_manager;
  nsCOMPtr<IEventSource> m_event_source;
  nsCOMPtr<IEventListener> m_el_passive;

  static QString machine_id_from_machine_event(IEvent* event);

  virtual void on_machine_state_changed(IEvent* event);
  virtual void on_machine_registered(IEvent* event);
  virtual void on_session_state_changed(IEvent* event);
  virtual void on_machine_event(IEvent* event);

  volatile bool m_event_listening;
  static void event_listener_th(CVBoxManagerLinux *manager);

public:
  virtual int init_machines(void);
  virtual int launch_vm(const QString& vm_id, vb_launch_mode_t lm = VBML_HEADLESS);
  virtual int turn_off(const QString& vm_id, bool save_state = false);
  virtual int pause(const QString &vm_id);
  virtual int resume(const QString &vm_id);
  virtual int remove(const QString &vm_id);
  virtual int add(const QString &vm_id);
  virtual QString version();
  virtual void shutdown_com();
};

#endif //VBOX_MANAGER_LINUX_H
