#ifndef CVBOXMANAGERWIN_H
#define CVBOXMANAGERWIN_H

#include "IVBoxManager.h"
#include <windows.h>
#include <atlbase.h>
#include <atlcom.h>

template <class T, class TParam = void *>
class ListenerImpl :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IEventListener, &IID_IEventListener, &LIBID_VirtualBox,             \
    kTypeLibraryMajorVersion, kTypeLibraryMinorVersion>
{
  T* mListener;
  CComPtr<IUnknown> m_pUnkMarshaler;

public:
  ListenerImpl() {
  }

  virtual ~ListenerImpl() {
  }

  HRESULT init(T* aListener, TParam param) {
    mListener = aListener;
    return mListener->init(param);
  }

  HRESULT init(T* aListener) {
    mListener = aListener;
    return mListener->init();
  }

  void uninit() {
    if (mListener) {
      mListener->uninit();
      delete mListener;
      mListener = 0;
    }
  }

  HRESULT FinalConstruct() {
    return CoCreateFreeThreadedMarshaler(this, &m_pUnkMarshaler.p);
  }

  void FinalRelease() {
    uninit();
    m_pUnkMarshaler.Release();
  }

  T* getWrapped() {
    return mListener;
  }

  DECLARE_NOT_AGGREGATABLE(ListenerImpl)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(ListenerImpl)
    COM_INTERFACE_ENTRY(IEventListener)
    COM_INTERFACE_ENTRY2(IDispatch, IEventListener)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
  END_COM_MAP()

  STDMETHOD(HandleEvent)(IEvent * aEvent)
  {
    VBoxEventType_T aType = VBoxEventType_Invalid;
    aEvent->get_Type(&aType);
    return mListener->HandleEvent(aType, aEvent);
  }
};

class CEventListenerWin {
private:
  IVBoxManager* m_vbox_manager;
public:
  CEventListenerWin(){}
  HRESULT init(IVBoxManager *manager) { m_vbox_manager = manager; return S_OK; }
  void uninit() {}

  /** Main event handler routine. */
  virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE HandleEvent(VBoxEventType_T e_type,
                                                                     IEvent *pEvent);
};

class CVBoxManagerWin : public IVBoxManager
{
private:
  friend class CVBoxManagerSingleton;

  CVBoxManagerWin();
  virtual ~CVBoxManagerWin();

  IVirtualBox* m_virtual_box;
  IEventSource* m_event_source;
  IEventListener* m_event_listener;

  static com::Bstr machine_id_from_machine_event(IEvent *event);
protected:
  virtual void on_machine_state_changed(IEvent *event);
  virtual void on_machine_registered(IEvent *event);
  virtual void on_session_state_changed(IEvent *event);
  virtual void on_machine_event(IEvent *event);  

public:
  virtual int init_machines();
  virtual int launch_vm(const com::Bstr &vm_id, vb_launch_mode_t lm);
  virtual int turn_off(const com::Bstr &vm_id, bool save_state);
  virtual int pause(const com::Bstr &vm_id);
  virtual int resume(const com::Bstr &vm_id);
  virtual int remove(const com::Bstr &vm_id);
  virtual int add(const com::Bstr &vm_id);
  virtual QString version();
};

typedef ListenerImpl<CEventListenerWin, IVBoxManager*> CEventListenerWinImpl;

#endif // CVBOXMANAGERWIN_H
