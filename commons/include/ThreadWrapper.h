#ifndef THREADWRAPPER_H
#define THREADWRAPPER_H

#ifndef RT_OS_WINDOWS
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
  int m_start_res;
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

  CThreadWrapper(T* lpExecutor, bool autoTerminate) : m_start_res(1), m_lpExecutor(lpExecutor), m_autoTerminateThread(autoTerminate){}
  ~CThreadWrapper(void){if (m_autoTerminateThread) Terminate(0);}

  /*!
   * \brief Starts executor's function in parallel thread
   */
  int Start(void) {
    m_start_res = pthread_create(&m_thread, NULL, CThreadWrapper::thread_func, (void*)m_lpExecutor);
    return m_start_res;
  }
  //////////////////////////////////////////////////////////////////////////

  /*!
   * \brief Blocks the calling thread until the thread represented by this instance terminates somehow
   */
  int Join(void) {
    if (m_start_res != 0) return m_start_res;
    return pthread_join(m_thread, NULL);
  }
  //////////////////////////////////////////////////////////////////////////

  /*!
   * \brief Blocks current thread until timeout elapses.
   */
  int Wait(int timeoutInMs) {
    (void)timeoutInMs;
//todo pthread_timed_join_np
    return 0;
  }

  /*!
   * \brief Terminates thread represented by current instance with `signal` code.
   * \param sig - return code.
   */
  int Terminate(int sig) {
    if (m_start_res != 0) return m_start_res;
    return pthread_kill(m_thread, sig);
  }
  //////////////////////////////////////////////////////////////////////////
};
#else

#include <windows.h>
#include <process.h>
#include <iostream>

/**
 * The CThreadWrapper class is used to wrap thread functions.
 * To run something in parallel thread create class Executor with
 * public void Run(void) method and copy constructor. After that
 * create instance of CThreadWrapper<ExecutorClass> instance(executorObject)
 * and call Start(). This wrapper allows use stack objects.
 */
template<class T> class CThreadWrapper {
private:
  HANDLE m_thread;
  T* m_lpExecutor;
  bool m_autoTerminate;
  //copy and assignment is prohibited
  CThreadWrapper(const CThreadWrapper&);
  void operator=(const CThreadWrapper&);

  static void thread_func(void *d) {
    ((T*)d)->Run();
  }

public:
  CThreadWrapper(T* lpExecutor, bool autoTerminate) : m_lpExecutor(lpExecutor), m_autoTerminate(autoTerminate){}

  ~CThreadWrapper(void){
    if (m_autoTerminate) {
      Terminate(0);
      Sleep(1000);
    }
  }

  int Start(void) {
    m_thread = (HANDLE) _beginthread(CThreadWrapper::thread_func, 0, (void*)m_lpExecutor);
    return (int)m_thread;
  }
  //////////////////////////////////////////////////////////////////////////

  int Join(void) {
    return WaitForSingleObject(m_thread, INFINITE);
  }
  //////////////////////////////////////////////////////////////////////////

  int Wait(int timeoutInMs) {
    return WaitForSingleObject(m_thread, timeoutInMs);
  }

  int Terminate(int sig) {
    return TerminateThread(m_thread, sig);
  }
  //////////////////////////////////////////////////////////////////////////
};

#endif

#endif // THREADWRAPPER_H
