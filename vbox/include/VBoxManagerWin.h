#ifndef CVBOXMANAGERWIN_H
#define CVBOXMANAGERWIN_H

#include "IVBoxManager.h"
#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>

class CVBoxManagerWin : public IVBoxManager
{
private:
  friend class CVBoxManagerSingleton;

  CVBoxManagerWin();
  virtual ~CVBoxManagerWin();

  IVirtualBox* m_virtual_box;
  IEventSource* m_event_source;
  IEventListener* m_el_passive;

  static com::Bstr machine_id_from_machine_event(IEvent *event);

  volatile bool m_event_listening;
  static void event_listener_th(CVBoxManagerWin *manager);
protected:
  virtual void on_machine_state_changed(IEvent *event);
  virtual void on_machine_registered(IEvent *event);
  virtual void on_session_state_changed(IEvent *event);
  virtual void on_machine_event(IEvent *event);  

public:
  virtual int init_machines();
  virtual int launch_vm(const com::Bstr &vm_id, vb_launch_mode_t lm);
  virtual int turn_off(const com::Bstr &vm_id, bool save_state);
  virtual int pause(const com::Bstr &vm_id);
  virtual int resume(const com::Bstr &vm_id);
  virtual int remove(const com::Bstr &vm_id);
  virtual int add(const com::Bstr &vm_id);
  virtual QString version();
  virtual void shutdown_com();
};

#endif // CVBOXMANAGERWIN_H
