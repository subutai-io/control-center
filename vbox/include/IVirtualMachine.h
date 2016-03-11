#ifndef IVIRTUALMACHINE
#define IVIRTUALMACHINE

#include "VBoxCommons.h"
#include <VBox/com/string.h>

class IVirtualMachine
{
protected:
  com::Bstr m_name;
  com::Bstr m_iid;
  IMachine* m_internal_machine;
  ISession* m_session;
  VM_State m_state;
  VM_SessionState m_session_state;

public:

  IVirtualMachine() :
    m_internal_machine(NULL),
    m_session(NULL),
    m_state(VMS_Null),
    m_session_state(VMSS_Null) {
  }
  virtual ~IVirtualMachine() {}

  const com::Bstr& name(void) const {return m_name;}
  const com::Bstr& id(void) const{return m_iid;}
  VM_State state(void) const {return m_state;}
  void set_state(VM_State state) {m_state = state;}
  void set_state(uint32_t state) {m_state = state >= VMS_UNDEFINED ? VMS_UNDEFINED : (VM_State)state;}

  virtual nsresult launch_vm(vb_launch_mode_t mode,
                             IProgress** progress) = 0;

  virtual nsresult save_state(IProgress** progress) = 0;
  virtual nsresult turn_off(IProgress** progress) = 0;
  virtual nsresult run_process(const char* path,
                               const char* user,
                               const char* password,
                               int argc,
                               const char** argv) = 0;
  virtual nsresult pause() = 0;
  virtual nsresult resume() = 0;
};

#endif // IVIRTUALMACHINE

