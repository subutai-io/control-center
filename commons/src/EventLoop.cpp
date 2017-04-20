#ifndef EVENT_LOOP_CPP
#define EVENT_LOOP_CPP

#include <stdio.h>
#include <QThread>
#include "EventLoop.h"

using namespace SynchroPrimitives;

template<class TMRE>
CEventLoop<TMRE>::CEventLoop(pf_on_handle_exception cbOnHandleException,
                             pf_on_handle_method_timeout cbOnHandleMethodTimeout,
                             pf_on_log cbOnLog,
                             unsigned int methodTimeout,
                             bool autoTerminate) : //initialization list
  m_autoTerminate(autoTerminate),
  m_onHandleException(cbOnHandleException),
  m_onHandleMethodTimeout(cbOnHandleMethodTimeout),
  m_onLog(cbOnLog),
  m_methodTimeout(methodTimeout),
  m_loopWorker(this),
  m_loopWorkerThread(&m_loopWorker, true)
{
}

template<class TMRE>
CEventLoop<TMRE>::~CEventLoop() {
  if (m_autoTerminate)
    return;

  m_loopWorker.m_isDisposing = true;
  m_loopWorkerThread.Join();
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void CEventLoop<TMRE>::Run() {
  m_loopWorkerThread.Start();
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void CEventLoop<TMRE>::InvokeActionAsync(IFunctor *functor) {
  functor->SetIsSynchronized(false);
  m_loopWorker.EnqueAction(functor);
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void CEventLoop<TMRE>::InvokeActionSync(IFunctor *functor,
                                        bool runInEventLoopsThread,
                                        unsigned int timeout) {

  functor->SetIsSynchronized(true);
  if (!runInEventLoopsThread)   {
    m_loopWorker.InvokeMethod_S(functor);
    return;
  }

  SynchroPrimitives::Locker lock(&m_loopWorker.m_csForEvents);
  std::string methodsInQueue = m_loopWorker.EnqueAction(functor);

  int waitResult = MRE_Wrapper<TMRE>::MRE_Wait(&m_loopWorker.m_mre, timeout);
  if (waitResult != 0) {
    InvokeOnLogCallback(methodsInQueue.c_str());
    InvokeHandleMethodTimeoutCallback(functor->MethodName());
  }

  MRE_Wrapper<TMRE>::MRE_Reset(&m_loopWorker.m_mre);
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void *CEventLoop<TMRE>::InvokeActionWithResult(IFunctor *functor,
                                               bool runInEventLoopsThread,
                                               unsigned int timeout) {
  functor->SetIsSynchronized(true);
  if (runInEventLoopsThread) {

    SynchroPrimitives::Locker lock(&m_loopWorker.m_csForEvents);
    std::string methodsInQueue = m_loopWorker.EnqueAction(functor);

    int waitResult = MRE_Wrapper<TMRE>::MRE_Wait(&m_loopWorker.m_mre, timeout);
    if (waitResult != 0) {
      InvokeOnLogCallback(methodsInQueue.c_str());
      InvokeHandleMethodTimeoutCallback(functor->MethodName());
      functor->SetTimeoutOccured(true);
      delete functor;
      return FAILED_METHOD_RESULT;
    }

    MRE_Wrapper<TMRE>::MRE_Reset(&m_loopWorker.m_mre);
  } //runInEventLoopsThread
  else {
    m_loopWorker.InvokeMethod_S(functor);
  }

  void *result = functor->GetExceptionOccured() ? FAILED_METHOD_RESULT : functor->GetResult();
  delete functor;
  return result;
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void CEventLoop<TMRE>::LoopWorker::InvokeMethod_S(IFunctor *action) {
  try {
    (*action)();
  }
  catch(std::exception&){
    action->SetExceptionOccured(true);
  }
  catch (...) {
    action->SetExceptionOccured(true);
  }
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
std::string CEventLoop<TMRE>::LoopWorker::EnqueAction(IFunctor *functor) {
  std::string res = "";
  SynchroPrimitives::Locker lock(&m_csForQueue);
  m_functorsQueue.push_back(functor);
  return res;
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
IFunctor *CEventLoop<TMRE>::LoopWorker::DequeAction() {
  SynchroPrimitives::Locker lock(&m_csForQueue);
  std::vector<IFunctor*>::iterator front = m_functorsQueue.begin();
  IFunctor* result = *front;
  m_functorsQueue.erase(front);
  return result;
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
bool CEventLoop<TMRE>::LoopWorker::QueueIsEmpty() {
  SynchroPrimitives::Locker lock(&m_csForQueue);
  bool result = m_functorsQueue.empty();
  return result;
}
//////////////////////////////////////////////////////////////////////////

template<class TMRE>
void CEventLoop<TMRE>::LoopWorker::Run(void) {
  while(!m_isDisposing) {
    QThread::currentThread()->msleep(1);
    while(!QueueIsEmpty()) {
      IFunctor* action = DequeAction();
      InvokeMethod_S(action);

      if (!action->GetIsSynchronized())
        delete action;
      else {
        if (!action->GetTimeoutOccured())
          MRE_Wrapper<TMRE>::MRE_Set(&m_mre);
      }
    }//while !QueueIsEmpty()
  }//for(;;)
}//Run
//////////////////////////////////////////////////////////////////////////

#endif
