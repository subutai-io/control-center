/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/extensions/transmngr/public/ipcITransactionObserver.idl
 */

#ifndef __gen_ipcITransactionObserver_h__
#define __gen_ipcITransactionObserver_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ipcITransactionObserver */
#define IPCITRANSACTIONOBSERVER_IID_STR "656c0a6a-5cb3-45ec-8cb6-e7678897f937"

#define IPCITRANSACTIONOBSERVER_IID \
  {0x656c0a6a, 0x5cb3, 0x45ec, \
    { 0x8c, 0xb6, 0xe7, 0x67, 0x88, 0x97, 0xf9, 0x37 }}

class NS_NO_VTABLE ipcITransactionObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCITRANSACTIONOBSERVER_IID)

  /**
    * This gets called when a Transaction has been sent from the 
    *   TransactionManager. If the data passed in needs to be stored
    *   for longer than the life of the method the observer needs
    *   to make a copy.
    *
    * @param aQueueID
    *        The queue from which the transaction originated
    *
    * @param aData
    *        The data to be sent.
    *
    * @param aDataLen
    *        The length of the data argument
    */
  /* void onTransactionAvailable (in unsigned long aQueueID, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
  NS_IMETHOD OnTransactionAvailable(PRUint32 aQueueID, const PRUint8 *aData, PRUint32 aDataLen) = 0;

  /**
    * Called after an application sends an Attach message to the
    *   Transaction Manager.
    *
    * @param aQueueID
    *        The client has been attached to the queue with this ID
    *
    * @param aStatus
    *        The status of the operation, as defined in tmUtils.h
    */
  /* void onAttachReply (in unsigned long aQueueID, in unsigned long aStatus); */
  NS_IMETHOD OnAttachReply(PRUint32 aQueueID, PRUint32 aStatus) = 0;

  /**
    * Called after an application sends a Detach message. Indicates 
    *   to the client that no more messages will be coming from the
    *   the TM to this client. Also, no messages posted from this
    *   client to the indicated queue will be accepted.
    *
    * @param aQueueID
    *        The client has been detached from the queue with this ID
    *
    * @param aStatus
    *        The status of the operation, as defined in tmUtils.h
    */
  /* void onDetachReply (in unsigned long aQueueID, in unsigned long aStatus); */
  NS_IMETHOD OnDetachReply(PRUint32 aQueueID, PRUint32 aStatus) = 0;

  /**
    * The reply from the TM indicating all messages have been removed
    *   from the queue indicated.
    *
    * @param aQueueID
    *        The queue that has been flushed.
    *
    * @param aStatus
    *        The status of the operation, as defined in tmUtils.h
    */
  /* void onFlushReply (in unsigned long aQueueID, in unsigned long aStatus); */
  NS_IMETHOD OnFlushReply(PRUint32 aQueueID, PRUint32 aStatus) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCITRANSACTIONOBSERVER \
  NS_IMETHOD OnTransactionAvailable(PRUint32 aQueueID, const PRUint8 *aData, PRUint32 aDataLen); \
  NS_IMETHOD OnAttachReply(PRUint32 aQueueID, PRUint32 aStatus); \
  NS_IMETHOD OnDetachReply(PRUint32 aQueueID, PRUint32 aStatus); \
  NS_IMETHOD OnFlushReply(PRUint32 aQueueID, PRUint32 aStatus); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCITRANSACTIONOBSERVER(_to) \
  NS_IMETHOD OnTransactionAvailable(PRUint32 aQueueID, const PRUint8 *aData, PRUint32 aDataLen) { return _to OnTransactionAvailable(aQueueID, aData, aDataLen); } \
  NS_IMETHOD OnAttachReply(PRUint32 aQueueID, PRUint32 aStatus) { return _to OnAttachReply(aQueueID, aStatus); } \
  NS_IMETHOD OnDetachReply(PRUint32 aQueueID, PRUint32 aStatus) { return _to OnDetachReply(aQueueID, aStatus); } \
  NS_IMETHOD OnFlushReply(PRUint32 aQueueID, PRUint32 aStatus) { return _to OnFlushReply(aQueueID, aStatus); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCITRANSACTIONOBSERVER(_to) \
  NS_IMETHOD OnTransactionAvailable(PRUint32 aQueueID, const PRUint8 *aData, PRUint32 aDataLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnTransactionAvailable(aQueueID, aData, aDataLen); } \
  NS_IMETHOD OnAttachReply(PRUint32 aQueueID, PRUint32 aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnAttachReply(aQueueID, aStatus); } \
  NS_IMETHOD OnDetachReply(PRUint32 aQueueID, PRUint32 aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnDetachReply(aQueueID, aStatus); } \
  NS_IMETHOD OnFlushReply(PRUint32 aQueueID, PRUint32 aStatus) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnFlushReply(aQueueID, aStatus); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcITransactionObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCITRANSACTIONOBSERVER

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcITransactionObserver)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void onTransactionAvailable (in unsigned long aQueueID, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
NS_IMETHODIMP _MYCLASS_::OnTransactionAvailable(PRUint32 aQueueID, const PRUint8 *aData, PRUint32 aDataLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAttachReply (in unsigned long aQueueID, in unsigned long aStatus); */
NS_IMETHODIMP _MYCLASS_::OnAttachReply(PRUint32 aQueueID, PRUint32 aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onDetachReply (in unsigned long aQueueID, in unsigned long aStatus); */
NS_IMETHODIMP _MYCLASS_::OnDetachReply(PRUint32 aQueueID, PRUint32 aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onFlushReply (in unsigned long aQueueID, in unsigned long aStatus); */
NS_IMETHODIMP _MYCLASS_::OnFlushReply(PRUint32 aQueueID, PRUint32 aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_ipcITransactionObserver_h__ */
