/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/extensions/lock/public/ipcILockService.idl
 */

#ifndef __gen_ipcILockService_h__
#define __gen_ipcILockService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ipcILockService */
#define IPCILOCKSERVICE_IID_STR "9f6dbe15-d851-4b00-912a-5ac0be88a409"

#define IPCILOCKSERVICE_IID \
  {0x9f6dbe15, 0xd851, 0x4b00, \
    { 0x91, 0x2a, 0x5a, 0xc0, 0xbe, 0x88, 0xa4, 0x09 }}

/**
 * This service provides named interprocess locking.
 */
class NS_NO_VTABLE ipcILockService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCILOCKSERVICE_IID)

  /**
     * Call this method to acquire a named interprocess lock.
     *
     * @param aLockName
     *        specifies the name of the lock
     * @param aWaitIfBusy
     *        wait for the lock to become available; otherwise, fail if lock
     *        is already held by some other process.
     */
  /* void acquireLock (in string aLockName, in boolean aWaitIfBusy); */
  NS_IMETHOD AcquireLock(const char *aLockName, PRBool aWaitIfBusy) = 0;

  /**
     * Call this method to release a named lock.
     *
     * @param aLockName
     *        specifies the name of the lock
     */
  /* void releaseLock (in string aLockName); */
  NS_IMETHOD ReleaseLock(const char *aLockName) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCILOCKSERVICE \
  NS_IMETHOD AcquireLock(const char *aLockName, PRBool aWaitIfBusy); \
  NS_IMETHOD ReleaseLock(const char *aLockName); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCILOCKSERVICE(_to) \
  NS_IMETHOD AcquireLock(const char *aLockName, PRBool aWaitIfBusy) { return _to AcquireLock(aLockName, aWaitIfBusy); } \
  NS_IMETHOD ReleaseLock(const char *aLockName) { return _to ReleaseLock(aLockName); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCILOCKSERVICE(_to) \
  NS_IMETHOD AcquireLock(const char *aLockName, PRBool aWaitIfBusy) { return !_to ? NS_ERROR_NULL_POINTER : _to->AcquireLock(aLockName, aWaitIfBusy); } \
  NS_IMETHOD ReleaseLock(const char *aLockName) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReleaseLock(aLockName); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcILockService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCILOCKSERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcILockService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void acquireLock (in string aLockName, in boolean aWaitIfBusy); */
NS_IMETHODIMP _MYCLASS_::AcquireLock(const char *aLockName, PRBool aWaitIfBusy)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void releaseLock (in string aLockName); */
NS_IMETHODIMP _MYCLASS_::ReleaseLock(const char *aLockName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_ipcILockService_h__ */
