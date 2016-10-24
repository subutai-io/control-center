#ifndef MRE_WRAPPER_H
#define MRE_WRAPPER_H

/*!
 * \brief Wraps manual reset event's functional.
 * Template parameter is OS specific wrapper.
 */
template <class T> class MRE_Wrapper {
private:
  MRE_Wrapper(void);
  MRE_Wrapper(const MRE_Wrapper&);
  void operator=(const MRE_Wrapper&);

public:
  /*!
   * \brief Initialize manual reset event
   * \param Pointer to manual reset event.
   */
  static int MRE_Init(T* lpMre){return T::MRE_Init(lpMre);}

  /*!
   * \brief Blocks current thread until the specified manual reset event receives signal or timeout occurs
   * \param Pointer to manual reset event
   * \param Timeout in milliseconds
   */
  static int MRE_Wait(T* lpMre, int timeInMs){return T::MRE_Wait(lpMre, timeInMs);}

  /*!
   * \brief Sets the state of event to signaled
   * \param Pointer to manual reset event
   */
  static int MRE_Set(T* lpMre){return T::MRE_Set(lpMre);}

  /*!
   * \brief Sets the state of event to nonsignaled.
   * \param Pointer to manual reset event
   */
  static int MRE_Reset(T* lpMre){return T::MRE_Reset(lpMre);}

  /*!
   * \brief Releases all resources used by the current instance of manual reset event
   * \param lpMre
   */
  static int MRE_Destroy(T* lpMre){return T::MRE_Destroy(lpMre);}
};

#endif // MRE_WRAPPER_H
