#include <QApplication>
#include <assert.h>
#include <iostream>
#include <thread>
#include <QDebug>
#include "VBoxManagerLinux.h"
#include "VirtualMachineLinux.h"
#include <VBox/com/com.h>
#include <nsString.h>
#include <QMessageBox>

using namespace com;

CVBoxManagerLinux::CVBoxManagerLinux() :
  m_service_manager(nsnull),
  m_virtual_box(nsnull),
  m_component_manager(nsnull),
  m_event_source(nsnull),
  m_el_active(nsnull) {
#define CHECK_RES(x) if (NS_FAILED(m_last_error)) { \
  m_last_vb_error = x;\
  std::cout << m_last_error << " " << m_last_vb_error << std::endl;\
  break;\
}
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

    m_el_active = new CEventListenerLinux(this);
    uint32_t* interested = new uint32_t[m_dct_event_handlers.size()];
    for (auto i = m_dct_event_handlers.cbegin(); i != m_dct_event_handlers.cend(); ++i) {
      *interested++ = i->first;
    }
    interested -= m_dct_event_handlers.size();

    m_last_error = m_event_source->RegisterListener(m_el_active,
                                                    m_dct_event_handlers.size(),
                                                    interested,
                                                    PR_TRUE);
    delete[] interested;
    CHECK_RES(VBE_REGISTER_LISTENER);
  } while (false); // run loop once.
#undef CHECK_RES
}

CVBoxManagerLinux::~CVBoxManagerLinux() {

  if (m_event_source && m_el_active)
    m_event_source->UnregisterListener(m_el_active);
  m_service_manager = nsnull;
  m_component_manager = nsnull;
  m_virtual_box = nsnull;
  m_el_active = nsnull;
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
  ISessionStateChangedEvent* ssc_event;
  nsresult rc = event->QueryInterface(ISessionStateChangedEvent::GetIID(), (void**)&ssc_event);
  if (NS_FAILED(rc)) {
    return;
  }

  uint32_t state;
  ssc_event->GetState(&state);
  com::Bstr str_machine_id = machine_id_from_machine_event(event);
  emit vm_session_state_changed(str_machine_id);
}
////////////////////////////////////////////////////////////////////////////

void CVBoxManagerLinux::on_machine_event(IEvent *event) {
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
    PRUnichar *vm_name;
    machines[count]->GetName(&vm_name);

    QString name = QString::fromUtf16((ushort*)vm_name);
    qDebug() << "machine name " << name << "\n";
//    if (!name.contains("subutai"))
//        continue;

    ISession* session;
    rc = m_component_manager->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                         nsnull,
                                                         NS_GET_IID(ISession),
                                                         (void**)&session);

    if (NS_FAILED(rc)) continue;

    IVirtualMachine* vm = new CVirtualMachineLinux(machines[count], session);
    m_dct_machines[vm->id()] = vm;
  } while (count != 0);

  std::thread t(StartEventThread, m_event_source, m_el_passive);
  t.detach();
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

  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();
  if (state == VMS_Aborted) {
    //return 1;//aborted machine will run
  }

  if (state == VMS_Stuck) {
    return 2;
  }

  if( (int)state >= 8 && (int)state <= 23 ) //transition
  {
    qDebug() << "transition state\n" ;
    return 4;
  }

  if( (int)state >= 5 && (int)state <= 19 )
  {
    qDebug() << "turned on already \n" ;
    return 3;//1;
  }

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->launch_vm(lm,
                                        getter_AddRefs(progress));

  if (NS_FAILED(rc))
    return 9;

  HANDLE_PROGRESS(vm_launch_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::turn_off(const com::Bstr &vm_id, bool save_state) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();
  if (state == VMS_Aborted) {
    return 11;//Impossible
  }

  if (state == VMS_Stuck) {
    return 2;
  }

  if( (int)state >= 8 && (int)state <= 23 ) //transition
  {
    qDebug() << "transition state\n" ;
    return 4;
  }

  if( (int)state < 5 )
  {
    qDebug() << "turned on already \n" ;
    return 5;//1;
  }

  if (save_state) {
    nsCOMPtr<IProgress> progress;
    rc = m_dct_machines[vm_id]->save_state(getter_AddRefs(progress));
    if (FAILED(rc)){
      return 10;
    }
    HANDLE_PROGRESS(vm_save_state_progress, progress);
  }

  nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->turn_off(getter_AddRefs(progress));
  if (NS_FAILED(rc)) {
    return 19;
  }

  HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return 0;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::pause(const com::Bstr &vm_id) {
// Machine can be Paused only from Running/Teleporting/LiveSnapShotting State = 5

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();
  if( (int)state != 5 && (int)state != 8 && (int)state != 9 )
  {
    qDebug() << "not in running state \n" ;
    return 6;//1;
  }


  //nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->pause();
  if (NS_FAILED(rc)) {
    return 19;
  }

  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}
////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::resume(const com::Bstr &vm_id) {
// Machine can be Paused only from Running/Teleporting/LiveSnapShotting State = 5

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;
  nsresult rc, state;
  state = m_dct_machines[vm_id]->state();
  if(  (int)state != 6  )
  {
    qDebug() << "not in paused state \n" ;
    return 6;//1;
  }


  //nsCOMPtr<IProgress> progress;
  rc = m_dct_machines[vm_id]->resume();
  if (NS_FAILED(rc)) {
    return 21;
  }

  //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}

////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::remove(const com::Bstr &vm_id) {
// Machine can be Removed if State < 5
    QMessageBox msg;
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Virtual machine will be removed!"));
    msg.setInformativeText(tr("Are You sure?"));
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Cancel);
    int mret = msg.exec();
    switch (mret) {
    case QMessageBox::No:
        qDebug() << "no pressed \n";
        return 23;
    case QMessageBox::Yes:
        qDebug() << "yes pressed \n";
        break;
    default:
        return 23;
    }
    qDebug() << "ok pressed go further \n";
    if (m_dct_machines.find(vm_id) == m_dct_machines.end())
        return 1; //no machine
    nsresult rc, state;
    state = m_dct_machines[vm_id]->state();
    if(  (int)state > 4  )
    {
        qDebug() << "not in off-line state \n" ;
        msg.setIcon(QMessageBox::Information);
        msg.setText(tr("Virtual machine can not be removed!"));
        msg.setInformativeText(tr("Power off machine first"));
        msg.setStandardButtons(QMessageBox::Ok);
        mret = msg.exec();

        return 6;//1;
    }

    nsCOMPtr<IProgress> progress;
    rc = m_dct_machines[vm_id]->remove(getter_AddRefs(progress));

    if (FAILED(rc)) {
        qDebug() << "failed to remove machine \n" ;
        return 23;
    }

    //HANDLE_PROGRESS(vm_turn_off_progress, progress);
    return rc;
}


////////////////////////////////////////////////////////////////////////////

int CVBoxManagerLinux::add(const com::Bstr &vm_id) {// we need not vm_id actually here
// Machine can be Removed if State < 5

//dialog?

//  nsCOMPtr<IProgress> progress;

  nsresult rc;
  nsCOMPtr<IMachine> newmachine;
  rc = m_virtual_box->CreateMachine(NULL,        /* settings file */
                                  NS_LITERAL_STRING("test-lin-add").get(),
                                  0, nsnull,   /* groups (safearray)*/
                                  nsnull,      /* ostype */
                                  nsnull,      /* create flags */
                                  getter_AddRefs(newmachine));
   if (NS_FAILED(rc))
   {
       return 23;
   }

   rc = newmachine->SetName(NS_ConvertUTF8toUTF16("test-new").get());
   rc = newmachine->SetMemorySize(128);

   rc = m_virtual_box->RegisterMachine(newmachine);
      if (NS_FAILED(rc))
      {
          qDebug() << "could not register machine! \n";
          return rc;
      }

//   rc = session->SaveSettings(); //need to save when going to be used!
//         if (NS_FAILED(rc)){
//            qDebug() << "could not save machine settings!\n";
//            return rc;
//         }
//    session->UnlockMachine();

   //HANDLE_PROGRESS(vm_turn_off_progress, progress);
  return rc;
}



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

//this thread needed just for calling VBox API. All actions are handled by active listener.
void CVBoxManagerLinux::StartEventThread(nsCOMPtr<IEventSource> eS,
                                         nsCOMPtr<IEventListener> eL) {

  static bool volatile terminateEThread = false;
  while (!terminateEThread) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    IEvent *event;
    eS->GetEvent( eL, 100, &event);

  }
}
////////////////////////////////////////////////////////////////////////////

