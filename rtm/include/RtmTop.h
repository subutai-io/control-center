#ifndef RTMTOP_H
#define RTMTOP_H

#include <string>
#include <stdint.h>

namespace realtime_monitoring {

#pragma pack(push)
#pragma pack(1)
  struct top_uptime {
/*current time
uptime of the machine
users sessions logged in
average load on the system
the 3 values refer to the last minute, five minutes and 15 minutes.*/
    std::string current_time;
    std::string uptime;
    uint32_t user_sessions_count;
    double al_1min, al_5min, al_15min;
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_task {
    uint32_t total, running, sleeping, stopped, zombie;
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_cpu {

/*Percentage of the CPU for user processes
Percentage of the CPU for system processes
Percentage of the CPU processes with priority upgrade nice
Percentage of the CPU not used
Percentage of the CPU processes waiting for I/O operations
Percentage of the CPU serving hardware interrupts (hi — Hardware IRQ)
Percentage of the CPU serving software interrupts (si — Software Interrupts)
The amount of CPU ‘stolen’ from this virtual machine by the hypervisor for other
tasks (such as running another virtual machine) this will be 0 on desktop
and server without Virtual machine. */
    double us, sy, ni, id, wa, hi, si, st;
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_memory {
    struct mem {
      uint32_t total, used, free, buffers;
    };
    struct swap {
      uint32_t total, used, free, cached;
    };
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_process {
    static const int LINUX_USER_MAX_LENGTH = 32;
    static const int LINUX_PROC_MAX_LENGTH = 16;
/*PID – l’ID of the process
USER – The user that is the owner of the process
PR – priority of the process
NI – The “NICE” value of the process
VIRT – virtual memory used by the process
RES – physical memory used from the process
SHR – shared memory of the process
S – indicates the status of the process: S=sleep R=running Z=zombie
%CPU – This is the percentage of CPU used by this process
%MEM – This is the percentage of RAM used by the process
TIME+ –This is the total time of activity of this process
COMMAND – And this is the name of the process */
    uint32_t pid;
    char user[LINUX_USER_MAX_LENGTH]; //32 is max length of user's name
    uint32_t pr, ni, virt, res, shr;
    char status;
    double cpu, mem;
    std::string time;
    char command[LINUX_PROC_MAX_LENGTH];
  };
  ////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

  class CTopRawParser {
  private:
  public:
  };
}

#endif // RTMTOP_H
