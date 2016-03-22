#ifndef VBOX_MANAGER_LINUX_H
#define VBOX_MANAGER_LINUX_H

#include <stdint.h>
#include <VBox/com/string.h>

#include "VBoxCommons.h"
#include "VirtualBoxHdr.h"
#include "IVBoxManager.h"

class CVBoxManagerLinux : public IVBoxManager
{
  class CEventListenerLinux : public IEventListener {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_IEVENTLISTENER
    NS_DECL_OWNINGTHREAD
    CEventListenerLinux(CVBoxManagerLinux* instance);
  private:
    CVBoxManagerLinux* m_instance;
    virtual ~CEventListenerLinux();
  };

 private:
  friend class CVBoxManagerSingleton;

  CVBoxManagerLinux();
  virtual ~CVBoxManagerLinux();

  nsCOMPtr<nsIEventQueue> m_eventQ;
  nsCOMPtr<nsIServiceManager> m_service_manager;
  nsCOMPtr<IVirtualBox> m_virtual_box;
  nsCOMPtr<nsIComponentManager> m_component_manager;
  nsCOMPtr<IEventSource> m_event_source;
  nsCOMPtr<IEventListener> m_el_active;
  nsCOMPtr<IEventListener> m_el_passive;

  static com::Bstr machine_id_from_machine_event(IEvent* event);

  virtual void on_machine_state_changed(IEvent* event);
  virtual void on_machine_registered(IEvent* event);
  virtual void on_session_state_changed(IEvent* event);
  virtual void on_machine_event(IEvent* event);

  static void StartEventThread(nsCOMPtr<IEventSource> eS, nsCOMPtr<IEventListener> eL);

public:
  virtual int init_machines(void);
  virtual int launch_vm(const com::Bstr& vm_id, vb_launch_mode_t lm = VBML_HEADLESS);
  virtual int turn_off(const com::Bstr& vm_id, bool save_state = false);
  virtual int pause(const com::Bstr &vm_id);
  virtual int resume(const com::Bstr &vm_id);
  virtual int remove(const com::Bstr &vm_id);
  virtual int add(const com::Bstr &vm_id);

};

#endif //VBOX_MANAGER_LINUX_H
