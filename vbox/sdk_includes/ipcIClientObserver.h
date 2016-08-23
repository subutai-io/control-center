/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/client/public/ipcIClientObserver.idl
 */

#ifndef __gen_ipcIClientObserver_h__
#define __gen_ipcIClientObserver_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ipcIClientObserver */
#define IPCICLIENTOBSERVER_IID_STR "42283079-030c-4b13-b069-a08b7ad5eab2"

#define IPCICLIENTOBSERVER_IID \
  {0x42283079, 0x030c, 0x4b13, \
    { 0xb0, 0x69, 0xa0, 0x8b, 0x7a, 0xd5, 0xea, 0xb2 }}

/**
 * ipcIClientObserver
 */
class NS_NO_VTABLE ipcIClientObserver : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCICLIENTOBSERVER_IID)

  enum { CLIENT_UP = 1U };

  enum { CLIENT_DOWN = 2U };

  /* void onClientStateChange (in unsigned long aClientID, in unsigned long aClientState); */
  NS_IMETHOD OnClientStateChange(PRUint32 aClientID, PRUint32 aClientState) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCICLIENTOBSERVER \
  NS_IMETHOD OnClientStateChange(PRUint32 aClientID, PRUint32 aClientState); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCICLIENTOBSERVER(_to) \
  NS_IMETHOD OnClientStateChange(PRUint32 aClientID, PRUint32 aClientState) { return _to OnClientStateChange(aClientID, aClientState); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCICLIENTOBSERVER(_to) \
  NS_IMETHOD OnClientStateChange(PRUint32 aClientID, PRUint32 aClientState) { return !_to ? NS_ERROR_NULL_POINTER : _to->OnClientStateChange(aClientID, aClientState); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcIClientObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCICLIENTOBSERVER

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcIClientObserver)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void onClientStateChange (in unsigned long aClientID, in unsigned long aClientState); */
NS_IMETHODIMP _MYCLASS_::OnClientStateChange(PRUint32 aClientID, PRUint32 aClientState)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_ipcIClientObserver_h__ */
