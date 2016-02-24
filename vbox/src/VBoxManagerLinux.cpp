#include <QApplication>
#include <assert.h>

#include "VBoxManagerLinux.h"
#include "VirtualMachineLinux.h"
#include <VBox/com/com.h>
#include <QDebug>

CVBoxManagerLinux::CVBoxManagerLinux() :
  m_service_manager(nsnull),
  m_virtual_box(nsnull),
  m_component_manager(nsnull),
  m_event_source(nsnull),
  m_event_listener(nsnull) {
#define CHECK_RES(x) if (NS_FAILED(m_last_error)) {m_last_vb_error = x; break;}
  do {
    m_last_error = com::Initialize(true);

    CHECK_RES(VBE_INIT_XPCOM2);

    m_last_error = NS_GetComponentManager(getter_AddRefs(m_component_manager));
    CHECK_RES(VBE_GET_COMPONENT_MANAGER);

    m_last_error = m_component_manager->CreateInstanceByContractID(NS_VIRTUALBOX_CONTRACTID,
                                                                   nsnull,
                                                                   NS_GET_IID(IVirtualBox),
                                                                   getter_AddRefs(m_virtual_box));
    CHECK_RES(VBE_CREATE_VIRTUAL_BOX);

    m_last_error = m_virtual_box->GetEventSource(getter_AddRefs(m_event_source));
    CHECK_RES(VBE_EVENT_SOURCE);

    m_event_listener = new CEventListenerLinux(this);
    uint32_t* interested = new uint32_t[m_dct_event_handlers.size()];
    for (auto i = m_dct_event_handlers.cbegin(); i != m_dct_event_handlers.cend(); ++i) {
      *interested++ = i->first;
    }
    interested -= m_dct_event_handlers.size();

    m_last_error = m_event_source->RegisterListener(m_event_listener,
                                                    m_dct_event_handlers.size(),
                                                    interested,
                                                    PR_TRUE);

//    m_last_error = m_event_source->RegisterListener(m_event_listener,
//                                                    m_dct_event_handlers.size(),
//                                                    interested,
//                                                    PR_FALSE);

    delete[] interested;
    CHECK_RES(VBE_REGISTER_LISTENER);
  } while (false); // run loop once.
#undef CHECK_RES
}

CVBoxManagerLinux::~CVBoxManagerLinux() {

  if (m_event_source && m_event_listener)
    m_event_source->UnregisterListener(m_event_listener);
  m_service_manager = nsnull;
  m_component_manager = nsnull;
  m_virtual_box = nsnull;
  m_event_listener = nsnull;
  m_event_source = nsnull;
  shutdown_com();
}
////////////////////////////////////////////////////////////////////////////

com::Bstr CVBoxManagerLinux::machine_id_from_machine_event(IEvent *event) {
  com::Bstr res;
  IMachineEvent* me_event;
  nsresult rc = event->QueryInterface(IMachineEvent::GetIID(), (void**)&me_event);
  if (NS_FAILED(rc)) return com::Bstr("");
  rc = me_event->GetMachineId(res.asOutParam());
  if (NS_FAILED(rc)) return com::Bstr("");
  return res;
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_state_changed(IEvent *event) {
    qDebug() << "on_machine_state_changed\n";
  IMachineStateChangedEvent* msc_event;
  event->QueryInterface(IMachineStateChangedEvent::GetIID(), (void**)&msc_event);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);
  uint32_t new_state;
  msc_event->GetState(&new_state);
  if (m_dct_machines.find(str_machine_id) != m_dct_machines.end())
    m_dct_machines[str_machine_id]->set_state(new_state); //because we can remove VM earlier
  emit vm_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_registered(IEvent *event) {
   qDebug() << "on_machine_registered\n";
  IMachineRegisteredEvent* mr_event;
  event->QueryInterface(IMachineRegisteredEvent::GetIID(), (void**)&mr_event);
  PRBool registered;
  mr_event->GetRegistered(&registered);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);

  if(registered != PR_TRUE) {
    if (m_dct_machines[str_machine_id])
      delete m_dct_machines[str_machine_id];
    m_dct_machines.erase(m_dct_machines.find(str_machine_id));
    emit vm_remove(str_machine_id);
    return;
  }

  IMachine *machine;
  nsresult rc;

  rc = m_virtual_box->FindMachine(str_machine_id.raw(), &machine);
  if (NS_FAILED(rc)) return;

  ISession* session;
  rc = m_component_manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                       nsnull,
                                                       NS_GET_IID(ISession),
                                                       (void**)&session);

  if (NS_FAILED(rc)) return;

  IVirtualMachine *vm = new CVirtualMachineLinux(machine, session);
  m_dct_machines[vm->id()] = vm;
  emit vm_add(vm->id());
}
////////////////////////////////////////////////////////////////////////////

//TODO check if we need this :)
void CVBoxManagerLinux::on_session_state_changed(IEvent *event) {
    qDebug() << "on_session_state_changed\n";
  ISessionStateChangedEvent* ssc_event;
  event->QueryInterface(ISessionStateChangedEvent::GetIID(), (void**)&ssc_event);
  uint32_t state;
  ssc_event->GetState(&state);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);
  emit vm_session_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_event(IEvent *event) {
     qDebug() << "on_machine_event";
  UNUSED_ARG(event);
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::init_machines() {
  uint32_t count;
  IMachine **machines;
  nsresult rc;

  if (m_last_vb_error != VBE_SUCCESS)
    return m_last_vb_error;

  rc = m_virtual_box->GetMachines(&count, &machines);
  if (NS_FAILED(rc)) return VBE_GET_MACHINES;

  do {
    --count;
    if (!machines[count]) continue;

    PRBool is_accesible = PR_FALSE;
    machines[count]->GetAccessible(&is_accesible);

    if (!is_accesible) continue;

    ISession* session;
    rc = m_component_manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                         nsnull,
                                                         NS_GET_IID(ISession),
                                                         (void**)&session);

    if (NS_FAILED(rc)) continue;

    IVirtualMachine* vm = new CVirtualMachineLinux(machines[count], session);
    session->UnlockMachine();
    m_dct_machines[vm->id()] = vm;
  } while (count != 0);

  return 0;
}
////////////////////////////////////////////////////////////////////////////

#define HANDLE_PROGRESS(sig, prog) do { \
    uint32_t percent = 0; \
    PRBool completed = PR_FALSE; \
    for (int i = 0; i < 10 && completed == PR_FALSE; ++i) { \
      rc = prog->WaitForCompletion(100); \
      prog->GetPercent(&percent); \
      prog->GetCompleted(&completed); \
      QApplication::processEvents(); \
      emit sig(vm_id, percent); \
    } \
  } while(0)

int CVBoxManagerLinux::launch_vm(const com::Bstr &vm_id,
                                 vb_launch_mode_t lm /*= VBML_HEADLESS*/) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;

  nsresult rc = NS_OK, state;
  state = m_dct_machines[vm_id]->state();
  if (state == VMS_Aborted) {
      qDebug() << "aborted\n";
      //return 1;//aborted machine will run
  }

  if (state == VMS_Stuck) {
      qDebug() << "stuck\n";
      return 2;
  }

  if( (int)state >= 8 && (int)state <= 23 ) //transition
  {
      qDebug() << "transition\n";
      return 4;//1;
  }

  if( (int)state >= 5 && (int)state <= 19 )//online
    {
      qDebug() << "online\n";
      return 3;//1;
  }


  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->launch_vm(lm,
                                        getter_AddRefs(progress));

  if (NS_FAILED(rc))
    return 9;

  HANDLE_PROGRESS(vm_launch_progress, progress);
  qDebug() << "turn on \n";
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::turn_off(const com::Bstr &vm_id, bool save_state) {

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;

  nsresult rc, state;


  state = m_dct_machines[vm_id]->state();
  if (state == VMS_Aborted) {
      //
//      IProgress* progress;
//      rc = m_dct_machines[vm_id]->launch_vm(VBML_HEADLESS, &progress);
//      if (FAILED(rc))
        qDebug() << "aborted\n";
        return 11;
  }

  state = m_dct_machines[vm_id]->state();

  if (state == VMS_Stuck) {
      qDebug() << "stuck\n";
      return 2;
  }

  if( (int)state >= 8 && (int)state <= 23 ) //transition
  {
      qDebug() << "transition\n";
      return 4;
  }

  if( (int)state >= 2 && (int)state <= 4 )
  {
      qDebug() << "power off\n";
      return 5;
  }

  if (save_state) {
    IProgress* progress;
    rc = m_dct_machines[vm_id]->save_state(&progress);
    if (FAILED(rc)) {
      qDebug() << "save state failed \n";
      return 10;
    }

    HANDLE_PROGRESS(vm_save_state_progress, progress);
  }

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->turn_off(getter_AddRefs(progress));
  if (NS_FAILED(rc)) {
    qDebug() << "turn off failed " << rc << "\n";
    return 19;
  }

  HANDLE_PROGRESS(vm_turn_off_progress, progress);
  qDebug() << "turn off \n";
  return 0;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

NS_IMPL_ISUPPORTS1(CVBoxManagerLinux::CEventListenerLinux, IEventListener)
CVBoxManagerLinux::CEventListenerLinux::CEventListenerLinux(CVBoxManagerLinux *instance) :
  m_instance(instance) {
}
////////////////////////////////////////////////////////////////////////////

CVBoxManagerLinux::CEventListenerLinux::~CEventListenerLinux(){

}
////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP CVBoxManagerLinux::CEventListenerLinux::HandleEvent(IEvent *event) {
  uint32_t e_type;
  event->GetType(&e_type);
  return m_instance->handle_event(e_type, event);
}
////////////////////////////////////////////////////////////////////////////
