/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/extensions/dconnect/public/ipcIDConnectService.idl
 */

#ifndef __gen_ipcIDConnectService_h__
#define __gen_ipcIDConnectService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ipcIDConnectService */
#define IPCIDCONNECTSERVICE_IID_STR "fe07ed16-2710-4a1e-a4e2-81302b62bf0e"

#define IPCIDCONNECTSERVICE_IID \
  {0xfe07ed16, 0x2710, 0x4a1e, \
    { 0xa4, 0xe2, 0x81, 0x30, 0x2b, 0x62, 0xbf, 0x0e }}

/**
 * This service provides methods similar to nsIComponentManager and
 * nsIServiceManager.  A ClientID parameter specifies the remote process
 * in which the object should live.
 *
 * ipcIService can be used to determine the ClientID of a remote process.
 *
 * It is assumed that both processes have access to the same typelibs.
 */
class NS_NO_VTABLE ipcIDConnectService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCIDCONNECTSERVICE_IID)

  /* void createInstance (in unsigned long aClientID, in nsCIDRef aClass, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
  NS_IMETHOD CreateInstance(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) = 0;

  /* void createInstanceByContractID (in unsigned long aClientID, in string aContractID, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
  NS_IMETHOD CreateInstanceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) = 0;

  /* void getService (in unsigned long aClientID, in nsCIDRef aClass, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
  NS_IMETHOD GetService(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) = 0;

  /* void getServiceByContractID (in unsigned long aClientID, in string aContractID, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
  NS_IMETHOD GetServiceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCIDCONNECTSERVICE \
  NS_IMETHOD CreateInstance(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result); \
  NS_IMETHOD CreateInstanceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result); \
  NS_IMETHOD GetService(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result); \
  NS_IMETHOD GetServiceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCIDCONNECTSERVICE(_to) \
  NS_IMETHOD CreateInstance(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) { return _to CreateInstance(aClientID, aClass, aIID, result); } \
  NS_IMETHOD CreateInstanceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) { return _to CreateInstanceByContractID(aClientID, aContractID, aIID, result); } \
  NS_IMETHOD GetService(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) { return _to GetService(aClientID, aClass, aIID, result); } \
  NS_IMETHOD GetServiceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) { return _to GetServiceByContractID(aClientID, aContractID, aIID, result); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCIDCONNECTSERVICE(_to) \
  NS_IMETHOD CreateInstance(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstance(aClientID, aClass, aIID, result); } \
  NS_IMETHOD CreateInstanceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->CreateInstanceByContractID(aClientID, aContractID, aIID, result); } \
  NS_IMETHOD GetService(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetService(aClientID, aClass, aIID, result); } \
  NS_IMETHOD GetServiceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetServiceByContractID(aClientID, aContractID, aIID, result); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcIDConnectService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCIDCONNECTSERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcIDConnectService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* void createInstance (in unsigned long aClientID, in nsCIDRef aClass, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
NS_IMETHODIMP _MYCLASS_::CreateInstance(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void createInstanceByContractID (in unsigned long aClientID, in string aContractID, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
NS_IMETHODIMP _MYCLASS_::CreateInstanceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getService (in unsigned long aClientID, in nsCIDRef aClass, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
NS_IMETHODIMP _MYCLASS_::GetService(PRUint32 aClientID, const nsCID & aClass, const nsIID & aIID, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void getServiceByContractID (in unsigned long aClientID, in string aContractID, in nsIIDRef aIID, [iid_is (aIID), retval] out nsQIResult result); */
NS_IMETHODIMP _MYCLASS_::GetServiceByContractID(PRUint32 aClientID, const char *aContractID, const nsIID & aIID, void * *result)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_ipcIDConnectService_h__ */
