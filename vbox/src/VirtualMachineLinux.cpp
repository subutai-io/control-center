#include <stdlib.h>
#include <nsXPCOM.h>
#include <nsIMemory.h>
#include <nsIServiceManager.h>
#include <nsIEventQueue.h>
#include <nsEventQueueUtils.h>
#include "VirtualMachineLinux.h"
#include "VBoxManagerLinux.h"
#include <QDebug>

CVirtualMachineLinux::CVirtualMachineLinux(IMachine *xpcom_machine, nsCOMPtr<nsIComponentManager> m_comp_man) {
  m_component_man = m_comp_man;
  m_internal_machine = xpcom_machine;
  xpcom_machine->GetName(m_name.asOutParam());
  xpcom_machine->GetId(m_iid.asOutParam());
  uint32_t state;
  xpcom_machine->GetState(&state);
  set_state(state);

  ISession *session1;
  nsresult rc;
  rc = m_comp_man->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                       nsnull,
                                                       NS_GET_IID(ISession),
                                                       (void**)&session1);

  if (NS_FAILED(rc))
      qDebug() << "create session " << rc << "\n";

  m_session = session1;
  m_session->UnlockMachine();
}
////////////////////////////////////////////////////////////////////////////

CVirtualMachineLinux::~CVirtualMachineLinux() {
  if (m_internal_machine != nsnull) {
    m_internal_machine->Release();
    m_internal_machine = nsnull;
    m_session = NULL;
    m_session->Release();

  }
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::launch_vm(vb_launch_mode_t mode,
                                         IProgress **progress)
{  
    nsresult rc, state;

    if (m_session == NULL){
         qDebug() << "session is " << "NULL\n";
         ISession *session1;
         nsresult rc;
         rc = m_component_man->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                              nsnull,
                                                              NS_GET_IID(ISession),
                                                              (void**)&session1);

         if (NS_FAILED(rc))
             qDebug() << "create session " << rc << "\n";

         m_session = session1;
    }
    rc = m_session->GetState(&state);
    if (NS_FAILED(rc)) {
        qDebug() << "turning on cannot get session state  " << state << "\n";
        return rc;
    }
    if (state == 2) {
     rc = m_session->UnlockMachine();
     if (NS_FAILED(rc)) {
       qDebug() << "turning on cannot unlock machine\n";
       return rc;
     }
   }
    rc = m_session->UnlockMachine();

    rc =  m_internal_machine->LaunchVMProcess(m_session,
                                             com::Bstr(CCommons::VM_launch_mode_to_str(mode)).raw(),
                                             com::Bstr("").raw(),
                                             progress);
    m_session->UnlockMachine();
    m_session = NULL;
    return rc;
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::save_state(IProgress **progress) {
  return m_internal_machine->SaveState(progress);
  m_session->UnlockMachine();
  m_session = NULL;
 }
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::turn_off(IProgress **progress) {
  qDebug() << "turning off \n";
   nsresult rc, state;
   if (m_session == NULL){
        qDebug() << "session is " << "NULL\n";
        ISession *session1;
        nsresult rc;
        rc = m_component_man->CreateInstanceByContractID(NS_SESSION_CONTRACTID,
                                                             nsnull,
                                                             NS_GET_IID(ISession),
                                                             (void**)&session1);

        if (NS_FAILED(rc))
            qDebug() << "create session " << rc << "\n";

        m_session = session1;

   }
   rc = m_session->GetState(&state);
   if (NS_FAILED(rc)) {
       qDebug() << "turning off cannot get session state  " << state << "\n";
       return rc;
   }
   qDebug() << "session state  " << state << "\n";
  if (state == 2) {
    rc = m_session->UnlockMachine();
    if (NS_FAILED(rc)) {
      qDebug() << "turning off cannot unlock machine\n";
      return rc;
    }
  }

  rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
  if (NS_FAILED(rc)) {
      qDebug() << "turning off cannot lock machine\n";
      return rc;
  }

  nsCOMPtr<IConsole> console;
  rc = m_session->GetConsole(getter_AddRefs(console));
  if (NS_FAILED(rc)) {
      rc = m_session->UnlockMachine();
      m_session = NULL;
      return rc;
  }

  rc = console->PowerDown(progress);
  if (NS_FAILED(rc)) {
      console = NULL;
      rc = m_session->UnlockMachine();
      m_session = NULL;
      return rc;
  }
  console = NULL;
  m_session->UnlockMachine();
  m_session = NULL;
  return rc;
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::run_process(const char *path,
                                           const char *user,
                                           const char *password,
                                           int argc,
                                           const char **argv) {

  nsresult rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
  nsCOMPtr<IConsole> console;
  m_session->GetConsole(getter_AddRefs(console));

  nsCOMPtr<IGuest> guest;
  rc = console->GetGuest(getter_AddRefs(guest));

  nsCOMPtr<IGuestSession> gsess;

  rc = guest->CreateSession(com::Bstr(user).raw(),
                                   com::Bstr(password).raw(),
                                   com::Bstr("").raw(), //domain is "". todo add param
                                   NULL,
                                   getter_AddRefs(gsess));


  uint32_t reason = 0;
  rc = gsess->WaitFor(GuestSessionWaitForFlag_Start, 10*1000, &reason);
  PRUnichar** env_changes;
  uint32_t env_count = 0;
  rc = gsess->GetEnvironmentChanges(&env_count, &env_changes);

  nsCOMPtr<IGuestProcess> gproc = nsnull;

  const PRUnichar** launch_args = new const PRUnichar*[argc];
  for (int i = 0; i < argc; ++i) {
    launch_args[i] = com::Bstr(argv[i]).raw();
  }

  uint32_t flags[] = {ProcessCreateFlag_WaitForProcessStartOnly};
  rc = gsess->ProcessCreate(com::Bstr(path).raw(),
                            argc,
                            launch_args,
                            env_count,
                            (const PRUnichar**)env_changes,
                            sizeof(flags) / sizeof(uint32_t),
                            flags,
                            5000,
                            getter_AddRefs(gproc));
  rc = gsess->Close();
  m_session->UnlockMachine();
  m_session = NULL;
  return rc;
}
////////////////////////////////////////////////////////////////////////////
