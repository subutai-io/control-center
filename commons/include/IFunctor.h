#ifndef IFUNCTOR_H
#define IFUNCTOR_H

#include <stddef.h>
#define FAILED_METHOD_RESULT      ((void*) -1)

/*!
 * \brief The IFunctor interface provides virtual methods for encapsulating functions (with or without result).
 */
class IFunctor {
protected:
  bool m_isSynchronized;
  bool m_exceptionOccured;
  bool m_timeoutOccured;
  const char* m_methodName;

public:
  explicit IFunctor(const char* methodName);
  virtual ~IFunctor(void){}

  /*!
   * \brief GetResult - virtual method for getting result from FunctorWithResult and FunctorWithoutResult
   * \return (void*) pointer to result. Use static_cast or another cast mechanism.
   */
  virtual void* GetResult(void) = 0;

  /*!
   * \brief operator () - like in all functors. Can be used like functor_object() or (*lp_functor_object)();
   */
  virtual void operator()(void) = 0;

  /*!
   * \brief Should this method be invoked in synchronous or asynchronous mode.
   */
  void SetIsSynchronized(bool sync) {m_isSynchronized = sync;}

  /*!
   * \brief Is method called in synchronous or asynchronous mode
   */
  bool GetIsSynchronized(void) const {return m_isSynchronized;}

  /*!
   * \brief Internal method name. Should be specified by developer
   */
  const char* MethodName(void) const {return m_methodName;}

  /*!
   * \brief This flag is set by EventLoop if exception occured
   */
  void SetExceptionOccured(bool val){m_exceptionOccured = val;}

  /*!
   * \brief Returns true if exception occured. Otherwise returns false
   */
  bool GetExceptionOccured(void) const {return m_exceptionOccured;}

  /*!
   * \brief This flag is set by EventLoop if method took too much time (specified by developer)
   */
  void SetTimeoutOccured(bool val){m_timeoutOccured = val;}

  /*!
   * \brief Returns true if timeout occured. Otherwise returns false
   */
  bool GetTimeoutOccured(void) const {return m_timeoutOccured;}
};

#endif // IFUNCTOR_H
