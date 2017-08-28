#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <pthread.h>
#include <signal.h>

class IRunnable {
private:
  pthread_t m_thread;  

  //copy and assignment is prohibited
  IRunnable(const IRunnable&);
  void operator=(const IRunnable&);

  static void *thread_func(void *d) {
    ((IRunnable*)d)->Run();
    return NULL;
  }

public:
  IRunnable(void) {}
  virtual ~IRunnable(void){}
  virtual void Run(void) = 0;

  int Start(void) {
    return pthread_create(&m_thread, NULL, IRunnable::thread_func, (void*)this);
  }
  //////////////////////////////////////////////////////////////////////////

  int Wait(void) {
    return pthread_join(m_thread, NULL);
  }
  //////////////////////////////////////////////////////////////////////////

  int Terminate(int sig) {
    return pthread_kill(m_thread, sig);
  }
  //////////////////////////////////////////////////////////////////////////
};

#endif // RUNNABLE_H
