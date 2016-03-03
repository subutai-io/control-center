#include "IVBoxManager.h"
#if defined(RT_OS_LINUX) || defined(RT_OS_DARWIN)
#include "VBoxManagerLinux.h"
#include "VirtualMachineLinux.h"
#elif RT_OS_WINDOWS
#include "VBoxManagerWin.h"
#include "VirtualMachineWin.h"
#define NS_FAILED FAILED
#endif
#include <VBox/com/com.h>

IVBoxManager* CVBoxManagerSingleton::p_instance = NULL;
CVBoxManagerSingleton::CVBoxManagerDestructor CVBoxManagerSingleton::p_destructor;

IVBoxManager::IVBoxManager() :
  m_last_error(0u),
  m_last_vb_error(VBE_SUCCESS) {
  init_event_handlers();
}
////////////////////////////////////////////////////////////////////////////

IVBoxManager::~IVBoxManager() {
  for (auto i = m_dct_machines.begin(); i != m_dct_machines.end(); ++i) {
    delete i->second;
  }
}
////////////////////////////////////////////////////////////////////////////

void IVBoxManager::init_event_handlers() {
  m_dct_event_handlers[VBoxEventType_OnMachineStateChanged] = &IVBoxManager::on_machine_state_changed;
  m_dct_event_handlers[VBoxEventType_OnMachineRegistered] = &IVBoxManager::on_machine_registered;
  m_dct_event_handlers[VBoxEventType_OnSessionStateChanged] = &IVBoxManager::on_session_state_changed;
}
////////////////////////////////////////////////////////////////////////////

int IVBoxManager::launch_process(const com::Bstr &vm_id,
                                 const char *path,
                                 const char *user,
                                 const char *password,
                                 const char **argv,
                                 int argc) {
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return 1;

  return m_dct_machines[vm_id]->run_process(path,
                                            user,
                                            password,
                                            argc,
                                            argv);
}
////////////////////////////////////////////////////////////////////////////

nsresult IVBoxManager::handle_event(VBoxEventType_T e_type, IEvent *event) {
  if (m_dct_event_handlers.find(e_type) !=
      m_dct_event_handlers.end()) {
    (this->*m_dct_event_handlers[e_type])(event);
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void IVBoxManager::init_com() {
//do nothing, because com initializes in constructor
}
////////////////////////////////////////////////////////////////////////////

void IVBoxManager::shutdown_com() {
  com::Shutdown();
}
////////////////////////////////////////////////////////////////////////////

IVBoxManager *CVBoxManagerSingleton::Instance() {
  if (p_instance == NULL) {
#if defined(RT_OS_LINUX) || defined(RT_OS_DARWIN)
    p_instance = new CVBoxManagerLinux;
    p_destructor.Init(p_instance);
#elif RT_OS_WINDOWS
    p_instance = new CVBoxManagerWin;
    p_destructor.Init(p_instance);
#endif
  }
  return p_instance;
}
////////////////////////////////////////////////////////////////////////////
