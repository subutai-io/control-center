#ifndef VBOXCOMMONS_H
#define VBOXCOMMONS_H

#if defined(RT_OS_LINUX) || defined(RT_OS_DARWIN)
#include <VirtualBox_XPCOM.h>
#include <nsXPCOM.h>
#include <nsIMemory.h>
#include <nsIServiceManager.h>
#include <nsIEventQueue.h>
#include <nsEventQueueUtils.h>
#elif RT_OS_WINDOWS
#include <VirtualBox.h>
typedef HRESULT nsresult;
#endif

#include <stdint.h>

/*
 * "gui": VirtualBox Qt GUI front-end
"headless": VBoxHeadless (VRDE Server) front-end
"sdl": VirtualBox SDL front-end
"emergencystop": reserved value, used for aborting the currently running VM or session owner.
 In this case the session parameter may be null (if it is non-null it isn't used in any way),
 and the progress return value will be always null. The operation completes immediately.
"": use the per-VM default frontend if set, otherwise the global default defined in the system properties.
 If neither are set, the API will launch a "gui" session, which may fail if there is no windowing environment
 available. See IMachine::defaultFrontend and ISystemProperties::defaultFrontend.
*/

typedef enum vb_launch_modes {
  VBML_GUI = 0,
  VBML_HEADLESS,
  VBML_EMERGENCY_STOP,
  VBML_DEFAULT
} vb_launch_mode_t;

////////////////////////////////////////////////////////////////////////////

typedef enum vb_errors {
  VBE_SUCCESS = 0,
  VBE_INIT_XPCOM2,
  VBE_GET_MAIN_EVENT_QUEUE,
  VBE_GET_COMPONENT_MANAGER,
  VBE_CREATE_VIRTUAL_BOX,
  VBE_EVENT_SOURCE,
  VBE_EVENT_LISTENER,
  VBE_REGISTER_LISTENER,
  VBE_EVENT_LISTENER_THREAD,
  VBE_GET_MACHINES,
  VBE_SAFE_ARRAY_ACCESS_DATA,
  VBE_TOTALLY_FAILED
} vb_errors_t;
////////////////////////////////////////////////////////////////////////////

class CVBoxCommons {
public:
  static const char* VM_session_state_to_str(uint32_t session_state) {
    static const char* str_session_state[] = {
      "Null", "Unlocked", "Locked", "Spawning", "Unlocking", "Undefined"
    };
    return str_session_state[session_state];
  }
  ////////////////////////////////////////////////////////////////////////////

  static const char* vm_state_to_str(MachineState_T state) {
    static const char* no_idea = "No idea";
    switch (state) {
      case MachineState_Null                    : return "Null";
      case MachineState_PoweredOff              : return "PoweredOff";
      case MachineState_Saved                   : return "Saved";
      case MachineState_Teleported              : return "Teleported";
      case MachineState_Aborted                 : return "Aborted";
      case MachineState_Running                 : return "Running";
      case MachineState_Paused                  : return "Paused";
      case MachineState_Stuck                   : return "Stuck";
      case MachineState_Teleporting             : return "Teleporting";
      case MachineState_LiveSnapshotting        : return "Live snapshotting";
      case MachineState_Starting                : return "Starting";
      case MachineState_Stopping                : return "Stopping";
      case MachineState_Saving                  : return "Saving";
      case MachineState_Restoring               : return "Restoring";
      case MachineState_TeleportingPausedVM     : return "Teleporting paused VM";
      case MachineState_TeleportingIn           : return "Teleporting in";
      case MachineState_FaultTolerantSyncing    : return "Fault tolerant syncing";
      case MachineState_DeletingSnapshotOnline  : return "Deleting snapshot online";
      case MachineState_DeletingSnapshotPaused  : return "Deleting snapshot paused";
      case MachineState_OnlineSnapshotting      : return "Online snapshotting";
      case MachineState_RestoringSnapshot       : return "Restoring snapshot";
      case MachineState_DeletingSnapshot        : return "Deleting snapshot";
      case MachineState_SettingUp               : return "Setting up";
      default: return no_idea;
    }
  }
  ////////////////////////////////////////////////////////////////////////////

  static const char* VM_launch_mode_to_str(vb_launch_mode_t mode) {
    static const char* vbml_str[] = {
      "gui",
      "headless",
      "sdl",
      "emergencystop",
      ""
    };
    return vbml_str[mode];
  }
  ////////////////////////////////////////////////////////////////////////////

  static const wchar_t* VM_lauch_mode_to_wstr(vb_launch_mode_t mode) {
    static const wchar_t* vbml_str[] = {
      L"gui",
      L"headless",
      L"sdl",
      L"emergencystop",
      L""
    };
    return vbml_str[mode];
  }
  ////////////////////////////////////////////////////////////////////////////

#ifdef RT_OS_WINDOWS
  static const BSTR VM_launch_mode_to_bstr(vb_launch_mode_t mode) {
    static const BSTR vbml_str[] = {
      ::SysAllocString(L"gui"),
      ::SysAllocString(L"headless"),
      ::SysAllocString(L"sdl"),
      ::SysAllocString(L"emergencystop"),
      ::SysAllocString(L"")
    };
    return vbml_str[mode];
  }
#endif

  static const char* VM_error_to_str(uint32_t code) {
    static const char* vb_err_str[] = {
        "SUCCESS = 0",
        "INIT_XPCOM2",
        "GET_MAIN_EVENT_QUEUE",
        "GET_COMPONENT_MANAGER",
        "CREATE_VIRTUAL_BOX",
        "EVENT_SOURCE",
        "EVENT_LISTENER",
        "REGISTER_LISTENER",
        "EVENT_LISTENER_THREAD",
        "GET_MACHINES",
        "SAFE_ARRAY_ACCESS_DATA",
        "TOTALLY_FAILED"
    };
    return code >= VBE_TOTALLY_FAILED ?
          vb_err_str[VBE_TOTALLY_FAILED] : vb_err_str[code];
  }
  ////////////////////////////////////////////////////////////////////////////

};

#endif // VBOXCOMMONS_H

