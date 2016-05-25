#ifndef IFUNCTOR_H
#define IFUNCTOR_H

#include <stddef.h>
#define FAILED_METHOD_RESULT      ((void*) -1)

/*!
 * \brief The IFunctor interface provides virtual methods for encapsulating methods
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
   * \brief SetIsSynchronized
   * \param withResult
   */
  void SetIsSynchronized(bool withResult) {m_isSynchronized = withResult;}

  /*!
   * \brief GetIsSynchronized
   * \return
   */
  bool GetIsSynchronized(void) const {return m_isSynchronized;}

  /*!
   * \brief MethodName
   * \return
   */
  const char* MethodName(void) const {return m_methodName;}

  /*!
   * \brief SetExceptionOccured
   * \param val
   */
  void SetExceptionOccured(bool val){m_exceptionOccured = val;}

  /*!
   * \brief GetExceptionOccured
   * \return
   */
  bool GetExceptionOccured(void) const {return m_exceptionOccured;}

  /*!
   * \brief SetTimeoutOccured
   * \param val
   */
  void SetTimeoutOccured(bool val){m_timeoutOccured = val;}

  /*!
   * \brief GetTimeoutOccured
   * \return
   */
  bool GetTimeoutOccured(void) const {return m_timeoutOccured;}
};

#endif // IFUNCTOR_H
