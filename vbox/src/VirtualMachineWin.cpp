#include <stdint.h>
#include <assert.h>

#include "VirtualMachineWin.h"

CVirtualMachineWin::CVirtualMachineWin(IMachine *com_machine) {
  assert(com_machine != NULL);
  m_internal_machine = com_machine;
  BSTR bstr_name, bstr_iid;
  m_internal_machine->get_Name(&bstr_name);
  m_internal_machine->get_Id(&bstr_iid);
  m_name = QString::fromUtf16((ushort*)bstr_name);
  m_iid = QString::fromUtf16((ushort*)bstr_iid);
  m_internal_machine->get_State(&m_state);

  CoCreateInstance(CLSID_Session,
                   NULL,                 /* no aggregation */
                   CLSCTX_INPROC_SERVER, /* the object lives in a server process on this machine */
                   IID_ISession,         /* IID of the interface */
                   (void**)&m_session);
}
////////////////////////////////////////////////////////////////////////////

CVirtualMachineWin::~CVirtualMachineWin() {  
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::launch_vm(vb_launch_mode_t mode,
                                       IProgress **progress) {

  m_session->UnlockMachine();
  nsresult rc = m_internal_machine->LaunchVMProcess(m_session,
                                                    CVBoxCommons::VM_launch_mode_to_bstr(mode),
                                                    NULL,
                                                    progress);

  return rc;
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::save_state(IProgress **progress) {
  return m_internal_machine->SaveState(progress);
}
////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::turn_off(IProgress **progress) {
  IConsole* console = NULL;
  m_session->UnlockMachine();
  nsresult rc = m_internal_machine->LockMachine(m_session, LockType_Shared);

  if (FAILED(rc)) return rc;

  rc = m_session->get_Console(&console);
  if (FAILED(rc)) return rc;

  rc = console->PowerDown(progress);
  console->Release();
  m_session->UnlockMachine();

  return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::pause() {
  nsresult rc;
  rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
  IConsole* console;
  m_session->get_Console(&console);
  rc = console->Pause();
  m_session->UnlockMachine();
  return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::resume() {
  nsresult rc;
  rc = m_internal_machine->LockMachine(m_session, LockType_Shared);
  IConsole* console;
  m_session->get_Console(&console);
  rc = console->Resume();
  m_session->UnlockMachine();
  return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::remove(IProgress **progress) {

  nsresult rc;
  BOOL accessible = FALSE;

  m_internal_machine->get_Accessible(&accessible);
  if (accessible == FALSE) {
    return 33;
  }
  SAFEARRAY *saMedia;
  rc = m_internal_machine->Unregister(CleanupMode_DetachAllReturnHardDisksOnly,//CleanupMode_Full,
                                      &saMedia);
  if (FAILED(rc)) return rc;

  rc = m_internal_machine->DeleteConfig(saMedia, progress);
  if (FAILED(rc)) return rc;

  return rc;
}

////////////////////////////////////////////////////////////////////////////

nsresult CVirtualMachineWin::run_process(const char *path,
                                         const char *user,
                                         const char *password,
                                         int argc,
                                         const char **argv)
{
  UNUSED_ARG(path);
  UNUSED_ARG(user);
  UNUSED_ARG(password);
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);
  return 0;
}
////////////////////////////////////////////////////////////////////////////
