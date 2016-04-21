#include <stdlib.h>
#include <nsXPCOM.h>
#include <nsIMemory.h>
#include <nsIServiceManager.h>
#include <nsIEventQueue.h>
#include <nsEventQueueUtils.h>
#include "VirtualMachineLinux.h"

CVirtualMachineLinux::CVirtualMachineLinux(IMachine *xpcom_machine,
                                           ISession* session) {
  m_internal_machine = xpcom_machine;
  xpcom_machine->GetName(m_name.asOutParam());
  xpcom_machine->GetId(m_iid.asOutParam());

  uint32_t state;
  xpcom_machine->GetState(&state);
  set_state(state);

  m_session = session;
}
////////////////////////////////////////////////////////////////////////////

CVirtualMachineLinux::~CVirtualMachineLinux() {
  if (m_internal_machine != nsnull) {
    m_internal_machine->Release();
    m_internal_machine = nsnull;
  }
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::launch_vm(vb_launch_mode_t mode,
                                         IProgress **progress)
{  
  return m_internal_machine->LaunchVMProcess(m_session,
                                             com::Bstr(CVBoxCommons::VM_launch_mode_to_str(mode)).raw(),
                                             com::Bstr("").raw(),
                                             progress);
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::save_state(IProgress **progress) {
  return m_internal_machine->SaveState(progress);
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::pause() {
      nsresult rc;
      rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
      nsCOMPtr<IConsole> console;
      m_session->GetConsole(getter_AddRefs(console));
      rc = console->Pause();
      m_session->UnlockMachine();
      return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::resume() {
      nsresult rc;
      rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
      nsCOMPtr<IConsole> console;
      m_session->GetConsole(getter_AddRefs(console));
      rc = console->Resume();
      m_session->UnlockMachine();
      return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::turn_off(IProgress **progress) {
  nsresult rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
  nsCOMPtr<IConsole> console;
  m_session->GetConsole(getter_AddRefs(console));
  rc = console->PowerDown(progress);
  m_session->UnlockMachine();
  return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineLinux::remove(IProgress **progress) {
  nsresult rc;
  IMedium **aMedia;
  PRUint32 cMedia;
  rc = m_internal_machine->Unregister((CleanupMode_T)CleanupMode_Full,  //DetachAllReturnHardDisksOnly,
                           &cMedia, &aMedia);
  if (NS_FAILED(rc)){
     //qDebug() << "Unregistering the machine failed! \n";
     return rc;
  }
//// Delete after fixing Removing vm files on MAC!//////////////////////////
#ifndef RT_OS_DARWIN
  rc = m_internal_machine->DeleteConfig(cMedia, aMedia, progress);
//          DeleteConfig(cMedia, aMedia, getter_AddRefs(progress));
#endif
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
  return rc;
}
////////////////////////////////////////////////////////////////////////////
