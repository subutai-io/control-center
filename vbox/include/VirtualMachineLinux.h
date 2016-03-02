#ifndef CVIRTUALMACHINE_H
#define CVIRTUALMACHINE_H

#include <stdint.h>
#include "IVirtualMachine.h"

class CVirtualMachineLinux : public IVirtualMachine
{
private:

  CVirtualMachineLinux(const CVirtualMachineLinux &vm);
  CVirtualMachineLinux& operator=(const CVirtualMachineLinux& vm);
  nsCOMPtr<nsIComponentManager> m_component_man;

public:
  //explicit CVirtualMachineLinux(IMachine* xpcom_machine, ISession *session);
  explicit CVirtualMachineLinux(IMachine* xpcom_machine, nsCOMPtr<nsIComponentManager> m_comp_man);
  virtual ~CVirtualMachineLinux(void);

  virtual nsresult launch_vm(vb_launch_mode_t mode,
                     IProgress** progress);

  virtual nsresult save_state(IProgress** progress);
  virtual nsresult turn_off(IProgress** progress);

  virtual nsresult run_process(const char* path,
                               const char* user,
                               const char* password,
                               int argc,
                               const char** argv);
};


#endif // CVIRTUALMACHINE_H
