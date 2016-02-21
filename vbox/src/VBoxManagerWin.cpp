#include <QApplication>
#include <stdint.h>
#include <comutil.h>
#include <VBox/com/com.h>
#include <VBox/com/ptr.h>
#include <VBox/com/array.h>

#include <QDebug>
#include "VBoxManagerWin.h"
#include "VirtualMachineWin.h"
typedef BOOL WINBOOL;

CVBoxManagerWin::CVBoxManagerWin() :
  m_virtual_box(NULL),
  m_event_source(NULL),
  m_event_listener(NULL)
{
#define CHECK_RES(x) if (FAILED(m_last_error)) { \
  qDebug() << QString("err : %1").arg(m_last_error, 1, 16); \
  m_last_vb_error = x; \
  break; \
}

  do {
    m_last_error = com::Initialize(true);
    CHECK_RES(VBE_INIT_XPCOM2);

    CComModule _Module;
    CAtlModule* _pAtlModule = &_Module;
    UNUSED_ARG(_pAtlModule);

    m_last_error = CoCreateInstance(CLSID_VirtualBox,
                                    NULL,
                                    CLSCTX_LOCAL_SERVER,
                                    IID_IVirtualBox,
                                    (void**)&m_virtual_box);
    CHECK_RES(VBE_CREATE_VIRTUAL_BOX);

    m_last_error = m_virtual_box->get_EventSource(&m_event_source);
    CHECK_RES(VBE_EVENT_SOURCE);    

//    m_last_error = m_event_source->CreateListener(&m_event_listener);
//    CHECK_RES(VBE_EVENT_LISTENER);

    ComObjPtr<CEventListenerWinImpl> pListener;
    pListener.createObject();
    pListener->init(new CEventListenerWin, this);

    /*why do we need this? comment for now*/
//    SAFEARRAYBOUND bound;
//    bound.lLbound = 0;
//    bound.cElements = (ULONG) m_dct_event_handlers.size();

    com::SafeArray<VBoxEventType_T> safe_interested;
    for (auto i = m_dct_event_handlers.cbegin(); i != m_dct_event_handlers.cend(); ++i) {
      safe_interested.push_back((VBoxEventType_T) i->first);
    }

    m_last_error = m_event_source->RegisterListener(pListener,
                                                    ComSafeArrayAsInParam(safe_interested),
                                                    TRUE);
    CHECK_RES(VBE_REGISTER_LISTENER);
  } while (false);
#undef CHECK_RES
}

CVBoxManagerWin::~CVBoxManagerWin() {
  if (m_event_source) {
    m_event_source->UnregisterListener(m_event_listener);
    m_event_source->Release();
  }
//  m_event_listener->Release();

  if (m_virtual_box) m_virtual_box->Release();

  m_event_source = NULL;
  m_event_listener = NULL;
  m_virtual_box = NULL;

  com::Shutdown();
}
////////////////////////////////////////////////////////////////////////////

com::Bstr CVBoxManagerWin::machine_id_from_machine_event(IEvent *event) {
  IMachineEvent* me_event;
  com::Bstr res;
  nsresult rc = event->QueryInterface(IID_IMachineEvent, (void**)&me_event);
  if (FAILED(rc)) return com::Bstr("");
  rc = me_event->get_MachineId(res.asOutParam());
  if (FAILED(rc)) return com::Bstr("");
  return res;
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerWin::on_machine_state_changed(IEvent *event) {
  IMachineStateChangedEvent* msc_event;
  event->QueryInterface(IID_IMachineStateChangedEvent, (void**)&msc_event);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);
  MachineState new_state;
  msc_event->get_State(&new_state);
  if (m_dct_machines.find(str_machine_id) != m_dct_machines.end())
    m_dct_machines[str_machine_id]->set_state((uint32_t)new_state); //because we can remove VM earlier
  emit vm_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerWin::on_machine_registered(IEvent *event) {
  IMachineRegisteredEvent* mr_event;
  event->QueryInterface(IID_IMachineRegisteredEvent, (void**)&mr_event);
  WINBOOL registered;
  mr_event->get_Registered(&registered);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);

  if(registered != TRUE) {
    if (m_dct_machines[str_machine_id])
      delete m_dct_machines[str_machine_id];
    m_dct_machines.erase(m_dct_machines.find(str_machine_id));
    emit vm_remove(str_machine_id);
    return;
  }

  IMachine *machine;
  nsresult rc;

  rc = m_virtual_box->FindMachine(str_machine_id.raw(), &machine);
  if (FAILED(rc)) return;

  IVirtualMachine *vm = new CVirtualMachineWin(machine);
  m_dct_machines[vm->id()] = vm;
  emit vm_add(vm->id());
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerWin::on_session_state_changed(IEvent *event) {
  ISessionStateChangedEvent* ssc_event;
  event->QueryInterface(IID_ISessionStateChangedEvent, (void**)&ssc_event);
  SessionState state;
  ssc_event->get_State(&state);  
  com::Bstr str_machine_id = machine_id_from_machine_event(event);
  emit vm_session_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerWin::on_machine_event(IEvent *event) {
  UNUSED_ARG(event);
  qDebug() << "on_machine_event";
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::init_machines() {
  IMachine **machines;
  nsresult rc;
  SAFEARRAY* safe_machines = NULL;
  uint32_t count;

  if (m_last_vb_error != VBE_SUCCESS)
    return m_last_vb_error;

  rc = m_virtual_box->get_Machines(&safe_machines);
  if (FAILED(rc)) return VBE_GET_MACHINES;

  count = safe_machines->rgsabound[0].cElements;
  rc = SafeArrayAccessData(safe_machines, (void**) &machines);
  if (FAILED(rc)) return VBE_SAFE_ARRAY_ACCESS_DATA;

  do {
    --count;
    if (!machines[count]) continue;
    WINBOOL accesible = FALSE;
    machines[count]->get_Accessible(&accesible);
    if (accesible == FALSE) continue;

    IVirtualMachine* vm = new CVirtualMachineWin(machines[count]);
    m_dct_machines[vm->id()] = vm;
  } while (count != 0);

  SafeArrayUnaccessData(safe_machines);
//  SafeArrayDestroy(safe_machines);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

#define HANDLE_PROGRESS(sig, prog) do { \
    ULONG percent = 0; \
    WINBOOL completed = FALSE; \
    for (int i = 0; i < 10 && completed == FALSE; ++i) { \
      rc = prog->WaitForCompletion(10); \
      prog->get_Percent(&percent); \
      prog->get_Completed(&completed); \
      QApplication::processEvents(); \
      emit sig(vm_id, (uint32_t)percent); \
    } \
  } while(0);

int CVBoxManagerWin::launch_vm(const com::Bstr &vm_id,
                               vb_launch_mode_t lm) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  rc = m_dct_machines[vm_id]->state();

//  if (state == VMS_Aborted) {
//      ;//aborted machine will run
//  }

  if (state == VMS_Stuck) {
      return 2;
  }

  if (  state == VMS_Running
              || state == VMS_Teleporting
              || state == VMS_LiveSnapshotting
              || state == VMS_Paused
              || state == VMS_TeleportingPausedVM
             ) {
      qDebug() << "turned on already \n" ;//+ vm_id;
      return 3;//1;
  }
  IProgress* progress = NULL;
  rc = m_dct_machines[vm_id]->launch_vm(lm,
                                        &progress);

  if (FAILED(rc)) {   
     return 9;
  }
  HANDLE_PROGRESS(vm_launch_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::turn_off(const com::Bstr &vm_id,
                              bool save_state) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  IProgress* progress;
  nsresult rc, state;
  rc = m_dct_machines[vm_id]->state();
  if (state == VMS_Aborted) {
      //
      rc = m_dct_machines[vm_id]->launch_vm(VBML_HEADLESS, &progress);
      if (FAILED(rc))
           return 11;
  }

  if (state == VMS_Stuck) {
      return 2;
  }

  if (  state == VMS_PoweredOff
              || state == VMS_Teleporting
              || state == VMS_LiveSnapshotting
              || state == VMS_Paused
              || state == VMS_TeleportingPausedVM
             ) {
      qDebug() << "turned on already \n" ;//+ vm_id;
      return 13;//1;
  }

  if (save_state) {
    IProgress* progress;
    rc = m_dct_machines[vm_id]->save_state(&progress);
    if (FAILED(rc)) {      
      return 19;
    }

    HANDLE_PROGRESS(vm_save_state_progress, progress);
  }

  rc = m_dct_machines[vm_id]->turn_off(&progress);
  if (FAILED(rc)) {
    return 15;
  }
  HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

HRESULT CEventListenerWin::HandleEvent(VBoxEventType e_type, IEvent *pEvent) {
  return m_vbox_manager->handle_event(e_type, pEvent);
}
