#ifndef VBOXCOMMONS_H
#define VBOXCOMMONS_H

#include <stdint.h>
#define UNUSED_ARG(x) ((void)x)
#include "VBoxCommonsPlatform.h"
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

  static const char* vm_state_to_str(uint32_t state) {
    static const char* str_machine_state[] = {
      "Null","Powered Off", "Saved",
      "Teleported", "Aborted", "Running",
      "Paused", "Stuck", "Teleporting",
      "Live Snapshotting", "Starting", "Stopping",
      "Saving", "Restoring", "Teleporting Paused VM",
      "Teleporting In", "Fault Tolerant Syncing", "Deleting Snapshot Online",
      "Deleting Snapshot Paused", "Online Snapshotting", "Restoring Snapshot",
      "Deleting Snapshot", "SettingUp", "Snapshotting"
    };
    return state >= 24 ? "undefined" : str_machine_state[state];
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

