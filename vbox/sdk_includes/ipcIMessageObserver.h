/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/client/public/ipcIMessageObserver.idl
 */

#ifndef __gen_ipcIMessageObserver_h__
#define __gen_ipcIMessageObserver_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ipcIMessageObserver */
#define IPCIMESSAGEOBSERVER_IID_STR "e40a4a3c-2dc1-470e-ab7f-5675fe1f1384"

#define IPCIMESSAGEOBSERVER_IID \
  {0xe40a4a3c, 0x2dc1, 0x470e, \
    { 0xab, 0x7f, 0x56, 0x75, 0xfe, 0x1f, 0x13, 0x84 }}

/**
 * ipcIMessageObserver
 */
class NS_NO_VTABLE ipcIMessageObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCIMESSAGEOBSERVER_IID)

  /**
     * @param aSenderID
     *        the client id of the sender of this message.  if sent by the
     *        daemon (or a deamon module), then this will have a value of 0.
     * @param aTarget
     *        the target of the message, corresponding to the target this
     *        observer was registered under.  this parameter is passed to allow
     *        an observer instance to receive messages for more than one target.
     * @param aData
     *        the data of the message.
     * @param aDataLen
     *        the data length of the message.
     */
  /* void onMessageAvailable (in unsigned long aSenderID, in nsIDRef aTarget, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
  NS_IMETHOD OnMessageAvailable(PRUint32 aSenderID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCIMESSAGEOBSERVER \
  NS_IMETHOD OnMessageAvailable(PRUint32 aSenderID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCIMESSAGEOBSERVER(_to) \
  NS_IMETHOD OnMessageAvailable(PRUint32 aSenderID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) { return _to OnMessageAvailable(aSenderID, aTarget, aData, aDataLen); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCIMESSAGEOBSERVER(_to) \
  NS_IMETHOD OnMessageAvailable(PRUint32 aSenderID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnMessageAvailable(aSenderID, aTarget, aData, aDataLen); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcIMessageObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCIMESSAGEOBSERVER

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcIMessageObserver)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void onMessageAvailable (in unsigned long aSenderID, in nsIDRef aTarget, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
NS_IMETHODIMP _MYCLASS_::OnMessageAvailable(PRUint32 aSenderID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_ipcIMessageObserver_h__ */
