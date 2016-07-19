#include <QApplication>
#include <assert.h>
#include <thread>
#include <QMessageBox>

#include "VBoxManagerLinux.h"
#include "VirtualMachineLinux.h"
#include "NotifiactionObserver.h"
#include "ApplicationLog.h"

CVBoxManagerLinux::CVBoxManagerLinux() :
  m_service_manager(nsnull),
  m_virtual_box(nsnull),
  m_component_manager(nsnull),
  m_event_source(nsnull),
  m_event_listening(false) {
#define CHECK_RES(x) if (NS_FAILED(m_last_error)) { \
  m_last_vb_error = x;\
  break;\
}
  do {
    m_last_error = NS_InitXPCOM2(getter_AddRefs(m_service_manager), nsnull, nsnull);
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

    m_last_error = m_event_source->CreateListener(getter_AddRefs(m_el_passive));
    CHECK_RES(VBE_EVENT_LISTENER);

    uint32_t* interested = new uint32_t[m_dct_event_handlers.size()];
    for (auto i = m_dct_event_handlers.cbegin(); i != m_dct_event_handlers.cend(); ++i) {
      *interested++ = i->first;
    }
    interested -= m_dct_event_handlers.size();
    m_last_error = m_event_source->RegisterListener(m_el_passive,
                                                    m_dct_event_handlers.size(),
                                                    interested,
                                                    PR_FALSE);
    delete[] interested;
    CHECK_RES(VBE_REGISTER_LISTENER);

    m_event_listening = true;
    std::thread th(event_listener_th, this);
    th.detach();
  } while (false); // run loop once.
#undef CHECK_RES
}

CVBoxManagerLinux::~CVBoxManagerLinux() {
  m_event_listening = false;
  while (!m_event_listening)
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

  if (m_event_source && m_el_passive)
    m_event_source->UnregisterListener(m_el_passive);

  m_service_manager = nsnull;
  m_component_manager = nsnull;
  m_virtual_box = nsnull;
  m_event_source = nsnull;
  m_el_passive = nsnull;

  shutdown_com();
}
////////////////////////////////////////////////////////////////////////////

QString CVBoxManagerLinux::machine_id_from_machine_event(IEvent *event) {
  PRUnichar* res;
  IMachineEvent* me_event;
  nsresult rc = event->QueryInterface(IMachineEvent::GetIID(), (void**)&me_event);
  if (NS_FAILED(rc)) return QString("");
  rc = me_event->GetMachineId(&res);
  if (NS_FAILED(rc)) return QString("");
  QString id = QString::fromUtf16((ushort*)res);
  return id;
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_state_changed(IEvent *event) {
  IMachineStateChangedEvent* msc_event;
  event->QueryInterface(IMachineStateChangedEvent::GetIID(), (void**)&msc_event);
  QString str_machine_id = machine_id_from_machine_event(event);
  uint32_t new_state;
  msc_event->GetState(&new_state);
  if (m_dct_machines.find(str_machine_id) != m_dct_machines.end())
    m_dct_machines[str_machine_id]->set_state(new_state); //because we can remove VM earlier
  emit vm_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_registered(IEvent *event) {
  IMachineRegisteredEvent* mr_event;
  event->QueryInterface(IMachineRegisteredEvent::GetIID(), (void**)&mr_event);
  PRBool registered;
  mr_event->GetRegistered(&registered);
  QString str_machine_id = machine_id_from_machine_event(event);

  if(registered != PR_TRUE) {
    if (m_dct_machines[str_machine_id])
      delete m_dct_machines[str_machine_id];
    m_dct_machines.erase(m_dct_machines.find(str_machine_id));
    emit vm_remove(str_machine_id);
    return;
  }

  IMachine *machine;
  nsresult rc;

  rc = m_virtual_box->FindMachine(str_machine_id.utf16(), &machine);
  if (NS_FAILED(rc)) return;

  PRBool accessible = PR_FALSE;
  machine->GetAccessible(&accessible);
  if (accessible == PR_FALSE) {
    return;
  }

  //todo use QString
  PRUnichar* vm_name;
  machine->GetName(&vm_name);

  QString name = QString::fromUtf16((ushort*)vm_name);
  if (!name.contains("subutai"))
      return;

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
  ISessionStateChangedEvent* ssc_event;
  nsresult rc = event->QueryInterface(ISessionStateChangedEvent::GetIID(), (void**)&ssc_event);
  if (NS_FAILED(rc)) {
    return;
  }

  uint32_t state;
  ssc_event->GetState(&state);
  QString str_machine_id = machine_id_from_machine_event(event);
  emit vm_session_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_event(IEvent *event) {
  UNUSED_ARG(event);
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::init_machines() {
  uint32_t count, rh_count = 0;
  IMachine **machines;
  nsresult rc;

  if (m_last_vb_error != VBE_SUCCESS)
    return m_last_vb_error;

  rc = m_virtual_box->GetMachines(&count, &machines);

  if (count == 0)
    return -1;

  if (NS_FAILED(rc)) return VBE_GET_MACHINES;

  do {
    --count;
    if (!machines[count]) continue;

    PRBool is_accesible = PR_FALSE;
    machines[count]->GetAccessible(&is_accesible);

    if (!is_accesible) continue;
    PRUnichar *vm_name;
    machines[count]->GetName(&vm_name);

    QString name = QString::fromUtf16((ushort*)vm_name);
    if (!name.contains("subutai"))
      continue;
    rh_count++;
    ISession* session;
    rc = m_component_manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                         nsnull,
                                                         NS_GET_IID(ISession),
                                                         (void**)&session);

    if (NS_FAILED(rc)) continue;

    IVirtualMachine* vm = new CVirtualMachineLinux(machines[count], session);
    m_dct_machines[vm->id()] = vm;
  } while (count != 0);
  return !rh_count;
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

int CVBoxManagerLinux::launch_vm(const QString &vm_id,
                                 vb_launch_mode_t lm /*= VBML_HEADLESS*/) {
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

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->launch_vm(lm, getter_AddRefs(progress));

  if (NS_FAILED(rc))
    return 3;

//  HANDLE_PROGRESS(vm_launch_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::turn_off(const QString &vm_id, bool save_state) {
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
    nsCOMPtr<IProgress> progress;
    rc = m_dct_machines[vm_id]->save_state(getter_AddRefs(progress));
    if (NS_FAILED(rc)){
      return 3;
    }
//    HANDLE_PROGRESS(vm_save_state_progress, progress);
  }

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->turn_off(getter_AddRefs(progress));
  if (NS_FAILED(rc)) {
    return 4;
  }

//  HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::pause(const QString &vm_id) {
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

  //nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->pause();
  if (NS_FAILED(rc)) {
    return 3;
  }

  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::resume(const QString &vm_id) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();
  if (state != VMS_Paused)
    return 2;

  //nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->resume();
  if (NS_FAILED(rc)) {
    return 3;
  }
  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}

////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::remove(const QString &vm_id) {
  // Machine can be Removed if State < 5
  QMessageBox msg;
  msg.setIcon(QMessageBox::Question);
#ifndef RT_OS_DARWIN
  msg.setText(tr("Virtual machine and all files will be removed!"));
#else
  msg.setText(tr("Virtual machine will be removed!Please delete files manually"));
#endif
  msg.setInformativeText(tr("Are You sure?"));
  msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  msg.setDefaultButton(QMessageBox::Cancel);
  if (msg.exec() == QMessageBox::No)
    return 1;

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 2; //no machine
  nsresult rc, state;
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
      msg.exec();
      return 2; //todo enum
  }

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->remove(getter_AddRefs(progress));
  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::add(const QString &vm_id) {
  UNUSED_ARG(vm_id);
  return NS_OK;
}
////////////////////////////////////////////////////////////////////////////

QString
CVBoxManagerLinux::version() {
  if (m_last_vb_error != VBE_SUCCESS) return "Undefined";
  PRUnichar* ver;
  nsresult rc = m_virtual_box->GetVersion(&ver);
  if (NS_SUCCEEDED(rc)) {
    QString result = QString::fromUtf16(ver);
    return result;
  }
  return "";
}

void
CVBoxManagerLinux::shutdown_com() {
  NS_ShutdownXPCOM(nsnull);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::event_listener_th(CVBoxManagerLinux* manager) {
  while (manager->m_event_listening) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    IEvent *event;
    nsresult rc = manager->m_event_source->GetEvent(manager->m_el_passive, 100, &event);

    if (NS_FAILED(rc)) {
      CApplicationLog::Instance()->LogError("Can't get event. rc = %x", rc);
      continue;
    }

    if (event == nsnull)
      continue;

    uint32_t type;
    rc = event->GetType(&type);
    if (NS_FAILED(rc)) {
      CApplicationLog::Instance()->LogError("Can't get event type. rc = %x", rc);
      continue;
    }

    if (manager->m_dct_event_handlers.find(type) == manager->m_dct_event_handlers.end())
      continue;

    //you wanna peace of me, boy?
    (manager->*manager->m_dct_event_handlers[type])(event);
  }
  manager->m_event_listening = true;
}
////////////////////////////////////////////////////////////////////////////

