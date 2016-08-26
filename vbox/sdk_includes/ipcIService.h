/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM /home/vbox/tinderbox/sdk/src/libs/xpcom18a4/ipc/ipcd/client/public/ipcIService.idl
 */

#ifndef __gen_ipcIService_h__
#define __gen_ipcIService_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class ipcIMessageObserver; /* forward declaration */

class ipcIClientObserver; /* forward declaration */


/* starting interface:    ipcIService */
#define IPCISERVICE_IID_STR "53d3e3a7-528f-4b09-9eab-9416272568c0"

#define IPCISERVICE_IID \
  {0x53d3e3a7, 0x528f, 0x4b09, \
    { 0x9e, 0xab, 0x94, 0x16, 0x27, 0x25, 0x68, 0xc0 }}

/**
 * ipcIService
 *
 * the IPC service provides the means to communicate with an external IPC
 * daemon and/or other mozilla-based applications on the same physical system.
 * the IPC daemon hosts modules (some builtin and others dynamically loaded)
 * with which applications may interact.
 *
 * at application startup, the IPC service will attempt to establish a
 * connection with the IPC daemon.  the IPC daemon will be automatically
 * started if necessary.  when a connection has been established, the IPC
 * service will enumerate the "ipc-startup-category" and broadcast an
 * "ipc-startup" notification using the observer service.
 *
 * when the connection to the IPC daemon is closed, an "ipc-shutdown"
 * notification will be broadcast.
 *
 * each client has a name.  the client name need not be unique across all
 * clients, but it is usually good if it is.  the IPC service does not require
 * unique names.  instead, the IPC daemon assigns each client a unique ID that
 * is good for the current "session."  clients can query other clients by name
 * or by ID.  the IPC service supports forwarding messages from one client to
 * another via the IPC daemon.
 *
 * for performance reasons, this system should not be used to transfer large
 * amounts of data.  instead, applications may choose to utilize shared memory,
 * and rely on the IPC service for synchronization and small message transfer
 * only.
 */
class NS_NO_VTABLE ipcIService : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(IPCISERVICE_IID)

  /**************************************************************************
     * properties of this process
     */
/**
     * returns the "client ID" assigned to this process by the IPC daemon.
     *
     * @throws NS_ERROR_NOT_AVAILABLE if no connection to the IPC daemon.
     */
  /* readonly attribute unsigned long ID; */
  NS_IMETHOD GetID(PRUint32 *aID) = 0;

  /**
     * this process can appear under several client names.  use the following
     * methods to add or remove names for this process.
     *
     * for example, the mozilla browser might have the primary name "mozilla",
     * but it could also register itself under the names "browser", "mail",
     * "news", "addrbook", etc.  other IPC clients can then query the IPC
     * daemon for the client named "mail" in order to talk with a mail program.
     *
     * XXX An IPC client name resembles a XPCOM contract ID.
     */
  /* void addName (in string aName); */
  NS_IMETHOD AddName(const char *aName) = 0;

  /* void removeName (in string aName); */
  NS_IMETHOD RemoveName(const char *aName) = 0;

  /**
     * add a new observer of client status change notifications.
     */
  /* void addClientObserver (in ipcIClientObserver aObserver); */
  NS_IMETHOD AddClientObserver(ipcIClientObserver *aObserver) = 0;

  /**
     * remove an observer of client status change notifications.
     */
  /* void removeClientObserver (in ipcIClientObserver aObserver); */
  NS_IMETHOD RemoveClientObserver(ipcIClientObserver *aObserver) = 0;

  /**************************************************************************
     * client query methods
     */
/**
     * resolve the given client name to the id of a connected client.  this
     * involves a round trip to the daemon, and as a result the calling thread
     * may block on this function call while waiting for the daemon to respond.
     */
  /* unsigned long resolveClientName (in string aName); */
  NS_IMETHOD ResolveClientName(const char *aName, PRUint32 *_retval) = 0;

  /**
     * tests whether a particular client is connected to the IPC daemon.
     */
  /* boolean clientExists (in unsigned long aClientID); */
  NS_IMETHOD ClientExists(PRUint32 aClientID, PRBool *_retval) = 0;

  /**************************************************************************
     * message methods
     */
/**
     * set a message observer for a particular message target.
     *
     * @param aTarget
     *        the message target being observed.  any existing observer will
     *        be replaced.
     * @param aObserver
     *        the message observer to receive incoming messages for the
     *        specified target.  pass null to remove the existing observer.
     * @param aOnCurrentThread
     *        if true, then the message observer will be called on the same
     *        thread that calls defineTarget.  otherwise, aObserver will be
     *        called on a background thread.
     */
  /* void defineTarget (in nsIDRef aTarget, in ipcIMessageObserver aObserver, in boolean aOnCurrentThread); */
  NS_IMETHOD DefineTarget(const nsID & aTarget, ipcIMessageObserver *aObserver, PRBool aOnCurrentThread) = 0;

  /**
     * send message asynchronously to a client or a module in the IPC daemon.
     * there is no guarantee that the message will be delivered.
     *
     * @param aClientID
     *        the client ID of the foreign application that should receive this
     *        message.  pass 0 to send a message to a module in the IPC daemon.
     * @param aTarget
     *        the target of the message.  if aClientID is 0, then this is the
     *        ID of the daemon module that should receive this message.
     * @param aData
     *        the message data.
     * @param aDataLen
     *        the message length.
     */
  /* void sendMessage (in unsigned long aReceiverID, in nsIDRef aTarget, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
  NS_IMETHOD SendMessage(PRUint32 aReceiverID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) = 0;

  /**
     * block the calling thread until a matching message is received.
     *
     * @param aSenderID
     *        pass 0 to wait for a message from the daemon.  pass PR_UINT32_MAX
     *        to wait for a message from any source.  otherwise, pass a client
     *        id to wait for a message from that particular client.
     * @param aTarget
     *        wait for a message to be delivered to this target.
     * @param aObserver
     *        this observer's OnMessageAvailable method is called when a
     *        matching message is available.  pass null to use the default
     *        observer associated with aTarget.
     * @param aTimeout
     *        indicates maximum length of time in milliseconds that this
     *        function may block the calling thread.
     *
     * @throws IPC_ERROR_WOULD_BLOCK if the timeout expires.
     *
     * the observer's OnMessageAvailable method may throw IPC_WAIT_NEXT_MESSAGE
     * to indicate that it does not wish to handle the message that it was
     * given, and that it will wait to be called with the next message.  this
     * enables the observer to keep messages in the queue that do not match the
     * desired message.  messages that remain in the queue will be dispatched
     * asynchronously to the default message handler after waitMessage finishes.
     *
     * NOTE: this function may hang the calling thread until a matching message
     * is received, so use it with caution.
     */
  /* void waitMessage (in unsigned long aSenderID, in nsIDRef aTarget, in ipcIMessageObserver aObserver, in unsigned long aTimeout); */
  NS_IMETHOD WaitMessage(PRUint32 aSenderID, const nsID & aTarget, ipcIMessageObserver *aObserver, PRUint32 aTimeout) = 0;

  /**
     * Call this method to disable the default message observer for a target.
     */
  /* void disableMessageObserver (in nsIDRef aTarget); */
  NS_IMETHOD DisableMessageObserver(const nsID & aTarget) = 0;

  /**
     * Call this method to re-enable the default message observer for a target.
     */
  /* void enableMessageObserver (in nsIDRef aTarget); */
  NS_IMETHOD EnableMessageObserver(const nsID & aTarget) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_IPCISERVICE \
  NS_IMETHOD GetID(PRUint32 *aID); \
  NS_IMETHOD AddName(const char *aName); \
  NS_IMETHOD RemoveName(const char *aName); \
  NS_IMETHOD AddClientObserver(ipcIClientObserver *aObserver); \
  NS_IMETHOD RemoveClientObserver(ipcIClientObserver *aObserver); \
  NS_IMETHOD ResolveClientName(const char *aName, PRUint32 *_retval); \
  NS_IMETHOD ClientExists(PRUint32 aClientID, PRBool *_retval); \
  NS_IMETHOD DefineTarget(const nsID & aTarget, ipcIMessageObserver *aObserver, PRBool aOnCurrentThread); \
  NS_IMETHOD SendMessage(PRUint32 aReceiverID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen); \
  NS_IMETHOD WaitMessage(PRUint32 aSenderID, const nsID & aTarget, ipcIMessageObserver *aObserver, PRUint32 aTimeout); \
  NS_IMETHOD DisableMessageObserver(const nsID & aTarget); \
  NS_IMETHOD EnableMessageObserver(const nsID & aTarget); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_IPCISERVICE(_to) \
  NS_IMETHOD GetID(PRUint32 *aID) { return _to GetID(aID); } \
  NS_IMETHOD AddName(const char *aName) { return _to AddName(aName); } \
  NS_IMETHOD RemoveName(const char *aName) { return _to RemoveName(aName); } \
  NS_IMETHOD AddClientObserver(ipcIClientObserver *aObserver) { return _to AddClientObserver(aObserver); } \
  NS_IMETHOD RemoveClientObserver(ipcIClientObserver *aObserver) { return _to RemoveClientObserver(aObserver); } \
  NS_IMETHOD ResolveClientName(const char *aName, PRUint32 *_retval) { return _to ResolveClientName(aName, _retval); } \
  NS_IMETHOD ClientExists(PRUint32 aClientID, PRBool *_retval) { return _to ClientExists(aClientID, _retval); } \
  NS_IMETHOD DefineTarget(const nsID & aTarget, ipcIMessageObserver *aObserver, PRBool aOnCurrentThread) { return _to DefineTarget(aTarget, aObserver, aOnCurrentThread); } \
  NS_IMETHOD SendMessage(PRUint32 aReceiverID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) { return _to SendMessage(aReceiverID, aTarget, aData, aDataLen); } \
  NS_IMETHOD WaitMessage(PRUint32 aSenderID, const nsID & aTarget, ipcIMessageObserver *aObserver, PRUint32 aTimeout) { return _to WaitMessage(aSenderID, aTarget, aObserver, aTimeout); } \
  NS_IMETHOD DisableMessageObserver(const nsID & aTarget) { return _to DisableMessageObserver(aTarget); } \
  NS_IMETHOD EnableMessageObserver(const nsID & aTarget) { return _to EnableMessageObserver(aTarget); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_IPCISERVICE(_to) \
  NS_IMETHOD GetID(PRUint32 *aID) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetID(aID); } \
  NS_IMETHOD AddName(const char *aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddName(aName); } \
  NS_IMETHOD RemoveName(const char *aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveName(aName); } \
  NS_IMETHOD AddClientObserver(ipcIClientObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->AddClientObserver(aObserver); } \
  NS_IMETHOD RemoveClientObserver(ipcIClientObserver *aObserver) { return !_to ? NS_ERROR_NULL_POINTER : _to->RemoveClientObserver(aObserver); } \
  NS_IMETHOD ResolveClientName(const char *aName, PRUint32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ResolveClientName(aName, _retval); } \
  NS_IMETHOD ClientExists(PRUint32 aClientID, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ClientExists(aClientID, _retval); } \
  NS_IMETHOD DefineTarget(const nsID & aTarget, ipcIMessageObserver *aObserver, PRBool aOnCurrentThread) { return !_to ? NS_ERROR_NULL_POINTER : _to->DefineTarget(aTarget, aObserver, aOnCurrentThread); } \
  NS_IMETHOD SendMessage(PRUint32 aReceiverID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen) { return !_to ? NS_ERROR_NULL_POINTER : _to->SendMessage(aReceiverID, aTarget, aData, aDataLen); } \
  NS_IMETHOD WaitMessage(PRUint32 aSenderID, const nsID & aTarget, ipcIMessageObserver *aObserver, PRUint32 aTimeout) { return !_to ? NS_ERROR_NULL_POINTER : _to->WaitMessage(aSenderID, aTarget, aObserver, aTimeout); } \
  NS_IMETHOD DisableMessageObserver(const nsID & aTarget) { return !_to ? NS_ERROR_NULL_POINTER : _to->DisableMessageObserver(aTarget); } \
  NS_IMETHOD EnableMessageObserver(const nsID & aTarget) { return !_to ? NS_ERROR_NULL_POINTER : _to->EnableMessageObserver(aTarget); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ipcIService
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPCISERVICE

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ipcIService)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* readonly attribute unsigned long ID; */
NS_IMETHODIMP _MYCLASS_::GetID(PRUint32 *aID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addName (in string aName); */
NS_IMETHODIMP _MYCLASS_::AddName(const char *aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeName (in string aName); */
NS_IMETHODIMP _MYCLASS_::RemoveName(const char *aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addClientObserver (in ipcIClientObserver aObserver); */
NS_IMETHODIMP _MYCLASS_::AddClientObserver(ipcIClientObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeClientObserver (in ipcIClientObserver aObserver); */
NS_IMETHODIMP _MYCLASS_::RemoveClientObserver(ipcIClientObserver *aObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* unsigned long resolveClientName (in string aName); */
NS_IMETHODIMP _MYCLASS_::ResolveClientName(const char *aName, PRUint32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean clientExists (in unsigned long aClientID); */
NS_IMETHODIMP _MYCLASS_::ClientExists(PRUint32 aClientID, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void defineTarget (in nsIDRef aTarget, in ipcIMessageObserver aObserver, in boolean aOnCurrentThread); */
NS_IMETHODIMP _MYCLASS_::DefineTarget(const nsID & aTarget, ipcIMessageObserver *aObserver, PRBool aOnCurrentThread)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void sendMessage (in unsigned long aReceiverID, in nsIDRef aTarget, [array, size_is (aDataLen), const] in octet aData, in unsigned long aDataLen); */
NS_IMETHODIMP _MYCLASS_::SendMessage(PRUint32 aReceiverID, const nsID & aTarget, const PRUint8 *aData, PRUint32 aDataLen)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void waitMessage (in unsigned long aSenderID, in nsIDRef aTarget, in ipcIMessageObserver aObserver, in unsigned long aTimeout); */
NS_IMETHODIMP _MYCLASS_::WaitMessage(PRUint32 aSenderID, const nsID & aTarget, ipcIMessageObserver *aObserver, PRUint32 aTimeout)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void disableMessageObserver (in nsIDRef aTarget); */
NS_IMETHODIMP _MYCLASS_::DisableMessageObserver(const nsID & aTarget)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void enableMessageObserver (in nsIDRef aTarget); */
NS_IMETHODIMP _MYCLASS_::EnableMessageObserver(const nsID & aTarget)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif

// category and observer event defines (XXX not yet implemented)
#define IPC_SERVICE_STARTUP_CATEGORY "ipc-startup-category"
#define IPC_SERVICE_STARTUP_TOPIC    "ipc-startup"
#define IPC_SERVICE_SHUTDOWN_TOPIC   "ipc-shutdown"

#endif /* __gen_ipcIService_h__ */
