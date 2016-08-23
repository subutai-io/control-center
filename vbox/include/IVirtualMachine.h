#ifndef IVIRTUALMACHINE
#define IVIRTUALMACHINE
#include <QString>
#include "VBoxCommons.h"

class IVirtualMachine
{
protected:
  QString m_name;
  QString m_iid;
  IMachine* m_internal_machine;
  ISession* m_session;
  MachineState_T m_state;
  SessionState_T m_session_state;

public:

  IVirtualMachine() :
    m_internal_machine(NULL),
    m_session(NULL),
    m_state(MachineState_Null),
    m_session_state(SessionState_Null) {
  }
  virtual ~IVirtualMachine() {    
    if (m_session) m_session->Release();
  }

  const QString& name(void) const {return m_name;}
  const QString& id(void) const{return m_iid;}
  MachineState_T state(void) const {return m_state;}
  void set_state(MachineState_T state) {m_state = state;}
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
  virtual nsresult remove(IProgress** progress) = 0;
};

#endif // IVIRTUALMACHINE

