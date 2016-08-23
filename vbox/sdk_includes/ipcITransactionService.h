/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/extensions/transmngr/public/ipcITransactionService.idl
 */

#ifndef __gen_ipcITransactionService_h__
#define __gen_ipcITransactionService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class ipcITransactionObserver; /* forward declaration */


/* starting interface:    ipcITransactionService */
#define IPCITRANSACTIONSERVICE_IID_STR "15561efb-8c58-4a47-813a-fa91cf730895"

#define IPCITRANSACTIONSERVICE_IID \
  {0x15561efb, 0x8c58, 0x4a47, \
    { 0x81, 0x3a, 0xfa, 0x91, 0xcf, 0x73, 0x08, 0x95 }}

class NS_NO_VTABLE ipcITransactionService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCITRANSACTIONSERVICE_IID)

  /** 
    * Connects the application to the transaction manager, defines the
    *   namespace and initializes internal storage
    *
    * @param aNamespace
    *        A string defining the scope of the transaction domains. It is
    *        used internally to seperate process listening to the same domain
    *        (ie. preferences) but for two different namespaces (ie. profile1 vs
    *        profile2).
    *
    * @returns NS_OK if all memory allocated properly and the IPC service was
    *          reached and attached to successfully.
    *
    * @returns an NS_ERROR_<foo> code specific to the failure otherwise
    */
  /* void init (in ACString aNamespace); */
  NS_IMETHOD Init(const nsACString & aNamespace) = 0;

  /** 
    * Links the observer passed in with the domain specified. This will allow
    *   the observer to post transactions dealing with this domain as well as
    *   receive transactions posted by other applications observing this
    *   domain.
    *
    * Return codes for this method confer information about the success of
    *   this call, not of the actual attaching of the observer to the domain.
    *   (except the TM_ERROR code - which means the observer can not attach)
    *   If the attach is successful the observer will have its OnAttachReply
    *   method called before this method returns.
    *
    * Note: This call is synchronous and will not return until the call to
    *       OnAttachReply is made.
    * 
    * @param aDomainName
    *        the name of the domain, in the current namespace, to listen for
    *        transactions from. i.e. cookies
    *
    * @param aObserver
    *        this will be used to notify the application when transactions
    *        and messages come in.
    *
    * @param aLockingCall
    *        Have the Transaction Sevice acquire a lock based on the domain
    *        before attaching. This should be used when persistant storage
    *        is being used to prevent data corruption.
    *
    * @returns NS_OK if the attach message was sent to the Transaction Manager.
    *
    * @returns an NS_ERROR_<foo> code specific to the failure otherwise
    *
    * @returns TM_ERROR_QUEUE_EXISTS if the queue already exists which means
    *          someone has already attached to it.
    */
  /* void attach (in ACString aDomainName, in ipcITransactionObserver aObserver, in PRBool aLockingCall); */
  NS_IMETHOD Attach(const nsACString & aDomainName, ipcITransactionObserver *aObserver, PRBool aLockingCall) = 0;

  /**
    * Sends a detach message to the Transaction Manager to unlink the observer
    *   associated with the domain passed in. 
    *
    * As in attach, return codes do not indicate success of detachment. The
    *   observer will have it's OnDetach method called if it is successfully
    *   detached.
    *
    * Note: This call is an asynchronous call.
    *
    * @param aDomainName
    *        the domain, in the current namespace, from which the client
    *        should be removed.
    *
    * @returns NS_OK if the detach message is sent to the Transaction Manager
    *
    * @returns NS_ERROR_FAILURE is something goes wrong
    *
    * @returns NS_ERRROR_UNEXPECTD if the domain does not have an observer 
    *          attached
    */
  /* void detach (in ACString aDomainName); */
  NS_IMETHOD Detach(const nsACString & aDomainName) = 0;

  /**
    * Sends a flush message to the Transaction Manager to remove all
    *   transactions for the domain. After this call there will be no
    *   transactions in the Transaction Manager for the namespace/domain
    *   pairing. It is up to the application to coordinate the flushing
    *   of the Transaction Manager with the writing of data to files,
    *   if needed.
    *
    * Note: This call is synchronous and will not return until the call to
    *       OnFlushReply is made.
    *
    * @param aDomainName
    *        The domain, in the current namespace, to flush.
    *
    * @param aLockingCall
    *        Have the Transaction Sevice acquire a lock based on the domain
    *        before flushing. This should be used when persistant storage
    *        is being used to prevent data corruption.
    *
    * @returns NS_OK if the flush message is sent to the Transaction Manager
    *
    * @returns NS_ERROR_FAILURE is something goes wrong
    *
    * @returns NS_ERRROR_UNEXPECTD if the domain does not have an observer 
    *          attached
    */
  /* void flush (in ACString aDomainName, in PRBool aLockingCall); */
  NS_IMETHOD Flush(const nsACString & aDomainName, PRBool aLockingCall) = 0;

  /**
    * Send the data to the Transaction Manager to be broadcast to any
    *   applications that have registered as observers of this particular
    *   namespace/domain pairing.
    *
    * If this domain is not being observed (attach has not been called for
    *   this domain) the message is queued until the attach is made and then
    *   the message is sent to the Transaction Manager with the proper domain
    *   information. 
    *
    *   XXXjg - this may not be neccessary with the synch attach call.
    *
    * Note: This call is an asynchronous call.
    *
    * @param aDomainName
    *        the domain, in the current namespace, to which the data will be
    *        sent.
    *
    * @param aData
    *        The actual data to be sent.
    *
    * @param aDataLen
    *        The length of the data argument
    */
  /* void postTransaction (in ACString aDomainName, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
  NS_IMETHOD PostTransaction(const nsACString & aDomainName, const PRUint8 *aData, PRUint32 aDataLen) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCITRANSACTIONSERVICE \
  NS_IMETHOD Init(const nsACString & aNamespace); \
  NS_IMETHOD Attach(const nsACString & aDomainName, ipcITransactionObserver *aObserver, PRBool aLockingCall); \
  NS_IMETHOD Detach(const nsACString & aDomainName); \
  NS_IMETHOD Flush(const nsACString & aDomainName, PRBool aLockingCall); \
  NS_IMETHOD PostTransaction(const nsACString & aDomainName, const PRUint8 *aData, PRUint32 aDataLen); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCITRANSACTIONSERVICE(_to) \
  NS_IMETHOD Init(const nsACString & aNamespace) { return _to Init(aNamespace); } \
  NS_IMETHOD Attach(const nsACString & aDomainName, ipcITransactionObserver *aObserver, PRBool aLockingCall) { return _to Attach(aDomainName, aObserver, aLockingCall); } \
  NS_IMETHOD Detach(const nsACString & aDomainName) { return _to Detach(aDomainName); } \
  NS_IMETHOD Flush(const nsACString & aDomainName, PRBool aLockingCall) { return _to Flush(aDomainName, aLockingCall); } \
  NS_IMETHOD PostTransaction(const nsACString & aDomainName, const PRUint8 *aData, PRUint32 aDataLen) { return _to PostTransaction(aDomainName, aData, aDataLen); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCITRANSACTIONSERVICE(_to) \
  NS_IMETHOD Init(const nsACString & aNamespace) { return !_to ? NS_ERROR_NULL_POINTER : _to->Init(aNamespace); } \
  NS_IMETHOD Attach(const nsACString & aDomainName, ipcITransactionObserver *aObserver, PRBool aLockingCall) { return !_to ? NS_ERROR_NULL_POINTER : _to->Attach(aDomainName, aObserver, aLockingCall); } \
  NS_IMETHOD Detach(const nsACString & aDomainName) { return !_to ? NS_ERROR_NULL_POINTER : _to->Detach(aDomainName); } \
  NS_IMETHOD Flush(const nsACString & aDomainName, PRBool aLockingCall) { return !_to ? NS_ERROR_NULL_POINTER : _to->Flush(aDomainName, aLockingCall); } \
  NS_IMETHOD PostTransaction(const nsACString & aDomainName, const PRUint8 *aData, PRUint32 aDataLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->PostTransaction(aDomainName, aData, aDataLen); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcITransactionService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCITRANSACTIONSERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcITransactionService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void init (in ACString aNamespace); */
NS_IMETHODIMP _MYCLASS_::Init(const nsACString & aNamespace)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void attach (in ACString aDomainName, in ipcITransactionObserver aObserver, in PRBool aLockingCall); */
NS_IMETHODIMP _MYCLASS_::Attach(const nsACString & aDomainName, ipcITransactionObserver *aObserver, PRBool aLockingCall)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void detach (in ACString aDomainName); */
NS_IMETHODIMP _MYCLASS_::Detach(const nsACString & aDomainName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void flush (in ACString aDomainName, in PRBool aLockingCall); */
NS_IMETHODIMP _MYCLASS_::Flush(const nsACString & aDomainName, PRBool aLockingCall)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void postTransaction (in ACString aDomainName, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
NS_IMETHODIMP _MYCLASS_::PostTransaction(const nsACString & aDomainName, const PRUint8 *aData, PRUint32 aDataLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// singleton implementing ipcITransactionService
#define IPC_TRANSACTIONSERVICE_CLASSNAME \
    "tmTransactionService"
#define IPC_TRANSACTIONSERVICE_CONTRACTID \
    "@mozilla.org/ipc/transaction-service;1"
#define IPC_TRANSACTIONSERVICE_CID \
{ /* 1403adf4-94d1-4c67-a8ae-d9f86972d378 */         \
  0x1403adf4,                                        \
  0x94d1,                                            \
  0x4c67,                                            \
  {0xa8, 0xae, 0xd9, 0xf8, 0x69, 0x72, 0xd3, 0x78}   \
}

#endif /* __gen_ipcITransactionService_h__ */
