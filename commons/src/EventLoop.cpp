#include <stdio.h>
#include "EventLoop.h"

using namespace SynchroPrimitives;

CEventLoop::CEventLoop(CEventLoop::pf_on_handle_exception cbOnHandleException,
                       CEventLoop::pf_on_handle_method_timeout cbOnHandleMethodTimeout,
                       CEventLoop::pf_on_log cbOnLog,
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

CEventLoop::~CEventLoop() {    
  if (m_autoTerminate)
    return;

   m_loopWorker.m_isDisposing = true;
   //todo
   //   if (m_loopWorkerThread.Wait(m_methodTimeout) != 0)
   //     m_loopWorkerThread.Terminate(-1);
   m_loopWorkerThread.Join();
}
//////////////////////////////////////////////////////////////////////////

void CEventLoop::Run() {
  m_loopWorkerThread.Start();
}
//////////////////////////////////////////////////////////////////////////

void CEventLoop::InvokeActionAsync(IFunctor *functor) {
  functor->SetIsSynchronized(false);
  m_loopWorker.EnqueAction(functor);
}
//////////////////////////////////////////////////////////////////////////

void CEventLoop::InvokeActionSync(IFunctor *functor,
                                  bool runInEventLoopsThread,
                                  unsigned int timeout) {

  functor->SetIsSynchronized(true);
  if (!runInEventLoopsThread)   {
    m_loopWorker.InvokeMethod_S(functor);
    return;
  }

  SynchroPrimitives::Locker lock(&m_loopWorker.m_csForEvents);
  std::string methodsInQueue = m_loopWorker.EnqueAction(functor);

  int waitResult = MRE_Wrapper<CLinuxManualResetEvent>::MRE_Wait(&m_loopWorker.m_mre, timeout);
  if (waitResult != 0) {
    InvokeOnLogCallback(methodsInQueue.c_str());
    InvokeHandleMethodTimeoutCallback(functor->MethodName());
  }

  MRE_Wrapper<CLinuxManualResetEvent>::MRE_Reset(&m_loopWorker.m_mre);
}
//////////////////////////////////////////////////////////////////////////

void *CEventLoop::InvokeActionWithResult(IFunctor *functor,
                                         bool runInEventLoopsThread,
                                         unsigned int timeout) {  
  functor->SetIsSynchronized(true);
  if (runInEventLoopsThread) {

    SynchroPrimitives::Locker lock(&m_loopWorker.m_csForEvents);
    std::string methodsInQueue = m_loopWorker.EnqueAction(functor);

    int waitResult = MRE_Wrapper<CLinuxManualResetEvent>::MRE_Wait(&m_loopWorker.m_mre, timeout);
    if (waitResult != 0) {
      InvokeOnLogCallback(methodsInQueue.c_str());
      InvokeHandleMethodTimeoutCallback(functor->MethodName());
      functor->SetTimeoutOccured(true);
      delete functor;
      return FAILED_METHOD_RESULT;
    }

    MRE_Wrapper<CLinuxManualResetEvent>::MRE_Reset(&m_loopWorker.m_mre);
  } //runInEventLoopsThread
  else {
    m_loopWorker.InvokeMethod_S(functor);
  }

  void *result = functor->GetExceptionOccured() ? FAILED_METHOD_RESULT : functor->GetResult();
  delete functor;
  return result;
}
//////////////////////////////////////////////////////////////////////////

void CEventLoop::LoopWorker::InvokeMethod_S(IFunctor *action) {
  try {
    (*action)();
  }
  catch(std::exception& exc){
    action->SetExceptionOccured(true);
  }
  catch (...) {
    action->SetExceptionOccured(true);
  }
}
//////////////////////////////////////////////////////////////////////////

std::string CEventLoop::LoopWorker::EnqueAction(IFunctor *functor) {
  std::string res = "";  
  SynchroPrimitives::Locker lock(&m_csForQueue);
  m_functorsQueue.push_back(functor);
  return res;
}
//////////////////////////////////////////////////////////////////////////

IFunctor *CEventLoop::LoopWorker::DequeAction() {
  SynchroPrimitives::Locker lock(&m_csForQueue);
  std::vector<IFunctor*>::iterator front = m_functorsQueue.begin();
  IFunctor* result = *front;  
  m_functorsQueue.erase(front);  
  return result;
}
//////////////////////////////////////////////////////////////////////////

bool CEventLoop::LoopWorker::QueueIsEmpty() {
  SynchroPrimitives::Locker lock(&m_csForQueue);
  bool result = m_functorsQueue.empty();  
  return result;
}
//////////////////////////////////////////////////////////////////////////

void CEventLoop::LoopWorker::Run(void) {
  while(!m_isDisposing) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    while(!QueueIsEmpty()) {
      IFunctor* action = DequeAction();
      InvokeMethod_S(action);

      if (!action->GetIsSynchronized())
        delete action;
      else {
        if (!action->GetTimeoutOccured())
          MRE_Wrapper<CLinuxManualResetEvent>::MRE_Set(&m_mre);
      }
    }//while !QueueIsEmpty()
  }//for(;;)
}//Run
//////////////////////////////////////////////////////////////////////////
