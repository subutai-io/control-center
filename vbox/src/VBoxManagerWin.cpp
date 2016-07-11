#include <QApplication>
#include <stdint.h>
#include <VBox/com/com.h>
#include <QMessageBox>

#include "VBoxManagerWin.h"
#include "VirtualMachineWin.h"
#include "NotifiactionObserver.h"
#include "ApplicationLog.h"

typedef BOOL WINBOOL;

CVBoxManagerWin::CVBoxManagerWin() :
  m_virtual_box(NULL),
  m_event_source(NULL),
  m_el_passive(NULL),
  m_event_listening(false)
{
#define CHECK_RES(x) if (FAILED(m_last_error)) { \
  CApplicationLog::Instance()->LogError("err : %x", m_last_error); \
  m_last_vb_error = x; \
  break; \
}

  do {
    m_last_error = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED /*| COINIT_SPEED_OVER_MEMORY*/);
    CHECK_RES(VBE_INIT_XPCOM2);

    m_last_error = CoCreateInstance(CLSID_VirtualBox,
                                    NULL,
                                    CLSCTX_LOCAL_SERVER,
                                    IID_IVirtualBox,
                                    (void**)&m_virtual_box);
    CHECK_RES(VBE_CREATE_VIRTUAL_BOX);

    m_last_error = m_virtual_box->get_EventSource(&m_event_source);
    CHECK_RES(VBE_EVENT_SOURCE);

    m_last_error = m_event_source->CreateListener(&m_el_passive);
    CHECK_RES(VBE_EVENT_LISTENER);

    SAFEARRAYBOUND bound = {(ULONG)(sizeof(VBoxEventType_T) * m_dct_event_handlers.size()), 0};
    SAFEARRAY *safe_interested = SafeArrayCreate(VT_I4, 1, &bound);

    if (safe_interested == NULL) {
      m_last_error = -1;
      CHECK_RES(VBE_EVENT_LISTENER);
    }

    VBoxEventType_T* ptr_interested;
    m_last_error = SafeArrayAccessData(safe_interested, (void**)&ptr_interested);
    CHECK_RES(VBE_EVENT_LISTENER);

    int ix = 0;
    for (auto i = m_dct_event_handlers.cbegin(); i != m_dct_event_handlers.cend(); ++i, ++ix) {
      ptr_interested[ix] = (VBoxEventType_T)i->first;
    }
    m_last_error = SafeArrayUnaccessData(safe_interested);
    CHECK_RES(VBE_EVENT_LISTENER);

    m_last_error = m_event_source->RegisterListener(m_el_passive,
                                                    safe_interested,
                                                    FALSE);
    CHECK_RES(VBE_REGISTER_LISTENER);
    m_event_listening = true;
    std::thread th(event_listener_th, this);
    th.detach();
  } while (false);
#undef CHECK_RES
}
////////////////////////////////////////////////////////

CVBoxManagerWin::~CVBoxManagerWin() {

  m_event_listening = false;
  while (!m_event_listening)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

  if (m_event_source) {
    m_event_source->UnregisterListener(m_el_passive);
    m_event_source->Release();
  }
  if (m_el_passive)
    m_el_passive->Release();

  if (m_virtual_box)
    m_virtual_box->Release();

  m_event_source = NULL;
  m_el_passive = NULL;
  m_virtual_box = NULL;
  shutdown_com();
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

  BOOL accessible = FALSE;
  machine->get_Accessible(&accessible);
  if (accessible == FALSE) {
    return;
  }

  BSTR vm_name;
  machine->get_Name(&vm_name);

  QString name = QString::fromUtf16((ushort*)vm_name);
  if (!name.contains("subutai"))
    return;

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
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::init_machines() {
  IMachine **machines;
  nsresult rc;
  SAFEARRAY* safe_machines = NULL;
  uint32_t count, rh_count = 0;

  if (m_last_vb_error != VBE_SUCCESS)
    return m_last_vb_error;

  rc = m_virtual_box->get_Machines(&safe_machines);
  if (FAILED(rc)) return VBE_GET_MACHINES;

  count = safe_machines->rgsabound[0].cElements;
  if (count == 0)
    return -1;

  rc = SafeArrayAccessData(safe_machines, (void**) &machines);
  if (FAILED(rc)) return VBE_SAFE_ARRAY_ACCESS_DATA;

  do {
    --count;
    if (!machines[count]) continue;
    WINBOOL accesible = FALSE;
    machines[count]->get_Accessible(&accesible);
    if (accesible == FALSE) continue;

    BSTR vm_name;
    machines[count]->get_Name(&vm_name);

    QString name = QString::fromUtf16((ushort*)vm_name);
    if (!name.contains("subutai"))
      continue;
    rh_count++;
    IVirtualMachine* vm = new CVirtualMachineWin(machines[count]);
    m_dct_machines[vm->id()] = vm;
  } while (count != 0);

  SafeArrayUnaccessData(safe_machines);
  //SafeArrayDestroy(safe_machines);
  return (rh_count == 0) ? 1 : 0;
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
  state = m_dct_machines[vm_id]->state();

  switch (state) {
    case VMS_Null:
    case VMS_PoweredOff:
    case VMS_Saved:
    case VMS_Teleported:
    case VMS_Aborted:
    case VMS_Running:
    case VMS_Paused:
      break;
    /*case VMS_Stuck:
    case VMS_Teleporting:
    case VMS_LiveSnapshotting:
    case VMS_Starting:
    case VMS_Stopping:
    case VMS_Saving:
    case VMS_Restoring:
    case VMS_TeleportingPausedVM:
    case VMS_TeleportingIn:
    case VMS_FaultTolerantSyncing:
    case VMS_DeletingSnapshotOnline:
    case VMS_DeletingSnapshotPaused:
    case VMS_OnlineSnapshotting:
    case VMS_RestoringSnapshot:
    case VMS_DeletingSnapshot:
    case VMS_SettingUp:
    case VMS_Snapshotting:
    case VMS_UNDEFINED:*/
    default:
      return 2; //todo enum
  }

  IProgress* progress = NULL;
  rc = m_dct_machines[vm_id]->launch_vm(lm, &progress);

  if (FAILED(rc)) {
    return 3;
  }
  HANDLE_PROGRESS(vm_launch_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::turn_off(const com::Bstr &vm_id,
                              bool save_state) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;

  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();

  switch (state) {
    case VMS_Running:
    case VMS_Paused:
    case VMS_Starting:
      break;
    /*case VMS_Null:
    case VMS_PoweredOff:
    case VMS_Saved:
    case VMS_Teleported:
    case VMS_Aborted:
    case VMS_Stuck:
    case VMS_Stopping:
    case VMS_LiveSnapshotting:
    case VMS_Saving:
    case VMS_Restoring:
    case VMS_Teleporting:
    case VMS_TeleportingPausedVM:
    case VMS_TeleportingIn:
    case VMS_FaultTolerantSyncing:
    case VMS_DeletingSnapshotOnline:
    case VMS_DeletingSnapshotPaused:
    case VMS_OnlineSnapshotting:
    case VMS_RestoringSnapshot:
    case VMS_DeletingSnapshot:
    case VMS_SettingUp:
    case VMS_Snapshotting:
    case VMS_UNDEFINED:*/
    default:
      return 2; //todo enum
  }

  if (save_state) {
    IProgress* progress;
    rc = m_dct_machines[vm_id]->save_state(&progress);
    if (FAILED(rc)){
      return 3;
    }
    HANDLE_PROGRESS(vm_save_state_progress, progress);
  }

  IProgress* progress;
  rc = m_dct_machines[vm_id]->turn_off(&progress);
  if (FAILED(rc)){
    return 4;
  }

  HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::pause(const com::Bstr &vm_id) {
  // Machine can be Paused only from Running/Teleporting/LiveSnapShotting State = 5

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();

  switch (state) {
    case VMS_Running:
    case VMS_Teleporting:
    case VMS_LiveSnapshotting:
      break;
    /*case VMS_Null:
    case VMS_Paused:
    case VMS_Starting:
    case VMS_PoweredOff:
    case VMS_Saved:
    case VMS_Teleported:
    case VMS_Aborted:
    case VMS_Stuck:
    case VMS_Stopping:
    case VMS_Saving:
    case VMS_Restoring:
    case VMS_TeleportingPausedVM:
    case VMS_TeleportingIn:
    case VMS_FaultTolerantSyncing:
    case VMS_DeletingSnapshotOnline:
    case VMS_DeletingSnapshotPaused:
    case VMS_OnlineSnapshotting:
    case VMS_RestoringSnapshot:
    case VMS_DeletingSnapshot:
    case VMS_SettingUp:
    case VMS_Snapshotting:
    case VMS_UNDEFINED:*/
    default:
      return 2; //todo enum
  }

  rc = m_dct_machines[vm_id]->pause();
  if (FAILED(rc)) {
    return 3;
  }

  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::resume(const com::Bstr &vm_id) {
  // Machine can be Paused only from Running/Teleporting/LiveSnapShotting State = 5

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();

  if (state != VMS_Paused)
    return 2;

  rc = m_dct_machines[vm_id]->resume();
  if (FAILED(rc)) {
    return 3;
  }

  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}

////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::remove(const com::Bstr &vm_id) {
  // Machine can be Removed if State < 5
  nsresult rc, state;
  QMessageBox msg;
  msg.setIcon(QMessageBox::Question);
  msg.setText("Virtual machine will be removed! ");
  msg.setInformativeText("Are You sure?");
  msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msg.setDefaultButton(QMessageBox::Cancel);
  if (msg.exec() == QMessageBox::No)
    return 1;

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 2;

  state = m_dct_machines[vm_id]->state();

  switch (state) {
    case VMS_Null:
    case VMS_PoweredOff:
    case VMS_Saved:
    case VMS_Teleported:
    case VMS_Aborted:
      break;

    case VMS_Running:
    case VMS_Paused:
    case VMS_Stuck:
    case VMS_Teleporting:
    case VMS_LiveSnapshotting:
    case VMS_Starting:
    case VMS_Stopping:
    case VMS_Saving:
    case VMS_Restoring:
    case VMS_TeleportingPausedVM:
    case VMS_TeleportingIn:
    case VMS_FaultTolerantSyncing:
    case VMS_DeletingSnapshotOnline:
    case VMS_DeletingSnapshotPaused:
    case VMS_OnlineSnapshotting:
    case VMS_RestoringSnapshot:
    case VMS_DeletingSnapshot:
    case VMS_SettingUp:
    case VMS_Snapshotting:
    case VMS_UNDEFINED:
    default:
      msg.setIcon(QMessageBox::Information);
      msg.setText(tr("Virtual machine can not be removed!"));
      msg.setInformativeText(tr("Power off machine first"));
      msg.setStandardButtons(QMessageBox::Ok);
      mret = msg.exec();
      return 2; //todo enum
  }

  IProgress* progress;
  rc = m_dct_machines[vm_id]->remove(&progress);  
  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}


////////////////////////////////////////////////////////////////////////////

int CVBoxManagerWin::add(const com::Bstr &vm_id) {
  UNUSED_ARG(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

QString CVBoxManagerWin::version() {
  com::Bstr ver("");
  nsresult rc = m_virtual_box->get_Version(ver.asOutParam());
  if (SUCCEEDED(rc)) {
    QString result((QChar*)ver.raw(), (int)ver.length());
    return result;
  }
  return "";
}
////////////////////////////////////////////////////////

void
CVBoxManagerWin::shutdown_com() {
  CoUninitialize();
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerWin::event_listener_th(CVBoxManagerWin* manager) {
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_SPEED_OVER_MEMORY);
  while (manager->m_event_listening) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    IEvent *event;
    nsresult rc = manager->m_event_source->GetEvent(manager->m_el_passive, 100, &event);

    if (FAILED(rc)) {
      CApplicationLog::Instance()->LogError("Can't get event. rc = %x", rc);
      continue;
    }

    if (event == NULL) {
      continue;
    }

    VBoxEventType type;
    rc = event->get_Type(&type);
    if (FAILED(rc)) {
      CApplicationLog::Instance()->LogError("Can't get event type. rc = %x", rc);
      continue;
    }

    if (manager->m_dct_event_handlers.find(type) == manager->m_dct_event_handlers.end())
      continue;

    //you wanna peace of me, boy?
    CApplicationLog::Instance()->LogInfo("New event : %d", type);
    (manager->*manager->m_dct_event_handlers[type])(event);
  }
  CoUninitialize();
  manager->m_event_listening = true;
}
////////////////////////////////////////////////////////////////////////////

