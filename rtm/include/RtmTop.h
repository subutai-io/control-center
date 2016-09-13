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
    top_uptime() : current_time(""), uptime(""),
      user_sessions_count(0), al_1min(0.0), al_5min(0.0), al_15min(0.0){}
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_task {
    uint32_t total, running, sleeping, stopped, zombie;
    top_task() : total(0), running(0), sleeping(0), stopped(0), zombie(0) {}
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
    top_cpu() :
      us(0.0), sy(0.0), ni(0.0), id(0.0),
      wa(0.0), hi(0.0), si(0.0), st(0.0) {}
  };
  ////////////////////////////////////////////////////////////////////////////

  struct top_memory {
    uint32_t total, used, free, buffers;
    top_memory() : total(0), used(0), free(0), buffers(0) {}
  };

  struct top_swap {
    uint32_t total, used, free, cached;
    top_swap() : total(0), used(0), free(0), cached(0) {}
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
    top_process() :
      pid(0), pr(0), ni(0), virt(0), res(0), shr(0),
      status('\0'), cpu(0.0), mem(0.0), time("") {

      for (int i = 0; i < LINUX_USER_MAX_LENGTH; ++i)
        user[i] = '\0';
      for (int i = 0; i < LINUX_PROC_MAX_LENGTH; ++i)
        command[i] = '\0';
    }
  };
  ////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

  class CTopRawParser {
  private:
    static top_uptime  parse_uptime(const std::string& str);
    static top_task    parse_task(const std::string& str);
    static top_cpu     parse_cpu(const std::string& str);
    static top_memory  parse_memory(const std::string& str);
    static top_swap    parse_swap(const std::string& str);
    static top_process parse_proc(const std::string& str);
  public:

    static void megatest();
  };
}

#endif // RTMTOP_H
