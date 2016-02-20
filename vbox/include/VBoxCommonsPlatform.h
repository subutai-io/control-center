#ifndef VBOXCOMMONSXP
#define VBOXCOMMONSXP

#include "VirtualBoxHdr.h"

enum VM_State {
  VMS_Null = MachineState_Null,
  VMS_PoweredOff = MachineState_PoweredOff,
  VMS_Saved = MachineState_Saved,
  VMS_Teleported = MachineState_Teleported,
  VMS_Aborted = MachineState_Aborted,
  VMS_Running = MachineState_Running,
  VMS_Paused = MachineState_Paused,
  VMS_Stuck = MachineState_Stuck,
  VMS_Teleporting = MachineState_Teleporting,
  VMS_LiveSnapshotting = MachineState_LiveSnapshotting,
  VMS_Starting = MachineState_Starting,
  VMS_Stopping = MachineState_Stopping,
  VMS_Saving = MachineState_Saving,
  VMS_Restoring = MachineState_Restoring,
  VMS_TeleportingPausedVM = MachineState_TeleportingPausedVM,
  VMS_TeleportingIn = MachineState_TeleportingIn,
  VMS_FaultTolerantSyncing = MachineState_FaultTolerantSyncing,
  VMS_DeletingSnapshotOnline = MachineState_DeletingSnapshotOnline,
  VMS_DeletingSnapshotPaused = MachineState_DeletingSnapshotPaused,
  VMS_OnlineSnapshotting = MachineState_OnlineSnapshotting,
  VMS_RestoringSnapshot = MachineState_RestoringSnapshot,
  VMS_DeletingSnapshot = MachineState_DeletingSnapshot,
  VMS_SettingUp = MachineState_SettingUp,
  VMS_Snapshotting = MachineState_Snapshotting,
  VMS_UNDEFINED
};

////////////////////////////////////////////////////////////////////////////

enum VM_SessionState {
  VMSS_Null = SessionState_Null,
  VMSS_Unlocked = SessionState_Unlocked,
  VMSS_Locked = SessionState_Locked,
  VMSS_Spawning = SessionState_Spawning,
  VMSS_Unlocking = SessionState_Unlocking,
  VMSS_UNDEFINED
};
////////////////////////////////////////////////////////////////////////////

#endif // VBOXCOMMONSXP

