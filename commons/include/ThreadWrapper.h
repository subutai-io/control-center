#ifndef THREADWRAPPER_H
#define THREADWRAPPER_H

#include <pthread.h>
#include <signal.h>

/**
 * The CThreadWrapper class is used to wrap thread functions.
 * To run something in parallel thread create class Executor with
 * public void Run(void) method and copy constructor. After that
 * create instance of CThreadWrapper<ExecutorClass> instance(executorObject)
 * and call Start(). This wrapper allows use stack objects.
 */
template<class T> class CThreadWrapper {
private:
  pthread_t m_thread;
  T* m_lpExecutor;
  bool m_autoTerminateThread;

  //copy and assignment is prohibited
  CThreadWrapper(void);
  CThreadWrapper(const CThreadWrapper&);
  void operator=(const CThreadWrapper&);

  static void* thread_func(void *d) {
    ((T*)d)->Run();
    return NULL;
  }

public:

  CThreadWrapper(T* lpExecutor, bool autoTerminate) : m_lpExecutor(lpExecutor), m_autoTerminateThread(autoTerminate){}
  ~CThreadWrapper(void){if (m_autoTerminateThread) Terminate(0);}

  int Start(void) {
    return pthread_create(&m_thread, NULL, CThreadWrapper::thread_func, (void*)m_lpExecutor);
  }
  //////////////////////////////////////////////////////////////////////////

  int Join(void) {
    return pthread_join(m_thread, NULL);
  }
  //////////////////////////////////////////////////////////////////////////

  int Wait(int timeoutInMs) {
//todo pthread_timed_join_np
    return 0;
  }

  int Terminate(int sig) {
    return pthread_kill(m_thread, sig);
  }
  //////////////////////////////////////////////////////////////////////////
};

#endif // THREADWRAPPER_H
