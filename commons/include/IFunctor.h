#ifndef IFUNCTOR_H
#define IFUNCTOR_H

#include <stddef.h>
#define FAILED_METHOD_RESULT      ((void*) -1)

class IFunctor {
protected:
  bool m_isSynchronized;
  bool m_exceptionOccured;
  bool m_timeoutOccured;
  const char* m_methodName;

public:
  explicit IFunctor(const char* methodName);
  virtual ~IFunctor(void){}

  virtual void* GetResult(void) = 0;
  virtual void operator()(void) = 0;

  void SetIsSynchronized(bool withResult) {m_isSynchronized = withResult;}
  bool GetIsSynchronized(void) const {return m_isSynchronized;}

  const char* MethodName(void) const {return m_methodName;}

  void SetExceptionOccured(bool val){m_exceptionOccured = val;}
  bool GetExceptionOccured(void) const {return m_exceptionOccured;}

  void SetTimeoutOccured(bool val){m_timeoutOccured = val;}
  bool GetTimeoutOccured(void) const {return m_timeoutOccured;}
};

#endif // IFUNCTOR_H
