#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <exception>
#include <map>
#include <vector>
#include <algorithm>

#include "IFunctor.h"
#include "EventLoopExceptionInfo.h"
#include "EventLoopException.h"
#include "InternalCriticalSection.h"
#include "Locker.h"
#include "MRE_Pthread.h"
#include "MRE_Wrapper.h"
#include "ThreadWrapper.h"

static const int METHOD_TIMEOUT = 10000;
template<class TMRE>
class CEventLoop
{
  typedef void (*pf_on_handle_exception)(std::exception& elExceptionInfo);
  typedef void (*pf_on_handle_method_timeout)(const char* methodName);
  typedef void (*pf_on_log)(const char* logMessage);

private:

  bool m_autoTerminate;
  pf_on_handle_exception m_onHandleException;
  pf_on_handle_method_timeout m_onHandleMethodTimeout;
  pf_on_log m_onLog;
  const unsigned int m_methodTimeout;

  void InvokeHandleExceptionCallback(std::exception& exc) {
    if (m_onHandleException != NULL) m_onHandleException(exc);
  }
  //////////////////////////////////////////////////////////////////////////

  void InvokeHandleMethodTimeoutCallback(const char* methodName) {
    if (m_onHandleMethodTimeout != NULL) m_onHandleMethodTimeout(methodName);
  }
  //////////////////////////////////////////////////////////////////////////

  void InvokeOnLogCallback(const char* logMessage) {
    if (m_onLog != NULL) m_onLog(logMessage);
  }
  //////////////////////////////////////////////////////////////////////////  

  class LoopWorker {
  private:    

    CEventLoop* m_eventLoop;
    std::vector<IFunctor*> m_functorsQueue;
    LoopWorker(void);
    LoopWorker(const LoopWorker&);
    void operator=(const LoopWorker&);

  public:
    bool m_isDisposing;
    SynchroPrimitives::CriticalSection m_csForQueue;
    SynchroPrimitives::CriticalSection m_csForEvents;
    TMRE m_mre;

    LoopWorker(CEventLoop* lpEL) : m_eventLoop(lpEL), m_isDisposing(false) {
      MRE_Wrapper<TMRE>::MRE_Init(&m_mre);
    }

    ~LoopWorker(){
      MRE_Wrapper<TMRE>::MRE_Destroy(&m_mre);
    }

    void Run(void);

    std::string EnqueAction(IFunctor* functor);
    IFunctor* DequeAction();
    bool QueueIsEmpty();
    void ClearActionQueue();
    void InvokeMethod_S(IFunctor* action);
  };
  //////////////////////////////////////////////////////////////////////////

  //DO NOT CHANGE ORDER!!!! of these 2 fields. MUST BE m_loopWorker and after m_loopWorkerThread
  LoopWorker m_loopWorker;
  CThreadWrapper<LoopWorker> m_loopWorkerThread;

public:

  CEventLoop(pf_on_handle_exception cbOnHandleException,
              pf_on_handle_method_timeout cbOnHandleMethodTimeout,
              pf_on_log cbOnLog,
              unsigned int methodTimeout,
              bool autoTerminate);

  ~CEventLoop(void);
  void Run(void);

  void  InvokeActionAsync(IFunctor* functor);
  void  InvokeActionSync(IFunctor* functor,
                         bool runInEventLoopsThread = true ,
                         unsigned int timeout = METHOD_TIMEOUT);
  void* InvokeActionWithResult(IFunctor* functor,
                               bool runInEventLoopsThread = true,
                               unsigned int timeout = METHOD_TIMEOUT);
  template<class RT> static RT GetSyncResult(CEventLoop* eventLoop,
                                             IFunctor* functor,
                                             bool runInEventLoopsThread) {

    const char* methodName = functor->MethodName();
    if (methodName == NULL) methodName = "Method without name";

    void* invResult = eventLoop->InvokeActionWithResult(functor, runInEventLoopsThread, eventLoop->m_methodTimeout);

    if (invResult == FAILED_METHOD_RESULT) {
      std::string msgDescription("Method with name {");
      msgDescription += std::string(methodName);
      msgDescription += std::string("} failed. For more information see cb_on_handle_exception handler.");

      eventLoop->InvokeOnLogCallback(msgDescription.c_str());
      throw CEventLoopException(msgDescription);
    }

    RT* pResult = (RT*) invResult;
    RT result = *pResult;
    delete pResult;
    return result;
  }
  //////////////////////////////////////////////////////////////////////////
};

/*because EventLoop is template class. And we wanted to separate definitions and implementations*/
#include "commons/src/EventLoop.cpp"

#endif // EVENTLOOP_H
