#ifndef VIRTUALMACHINEWIN_H
#define VIRTUALMACHINEWIN_H

#include "IVirtualMachine.h"

class CVirtualMachineWin : public IVirtualMachine
{
private:
  CVirtualMachineWin(void);
  CVirtualMachineWin(const CVirtualMachineWin &vm);
  CVirtualMachineWin& operator=(const CVirtualMachineWin& vm);

public:
  explicit CVirtualMachineWin(IMachine* com_machine);
  virtual ~CVirtualMachineWin(void);

  // IVirtualMachine interface
  nsresult launch_vm(vb_launch_mode_t mode,
                     IProgress **progress);
  nsresult save_state(IProgress **progress);
  nsresult turn_off(IProgress **progress);

  virtual nsresult run_process(const char *path, const char *user, const char *password, int argc, const char **argv);
};

#endif // VIRTUALMACHINEWIN_H
