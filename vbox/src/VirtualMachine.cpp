#include "VirtualMachine.h"

CVirtualMachine::CVirtualMachine(const QString &id,
                                 const QString &name) :
  m_id(id),
  m_name(name),
  m_state(MachineState_Null) {
}

CVirtualMachine::~CVirtualMachine() {
}

const char *
CVirtualMachine::vm_state_to_str(MachineState_T state) {
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

MachineState_T
CVirtualMachine::vm_state_from_str(const QString &str) {
  if (str.toLower().indexOf("null") != -1)
    return MachineState_Null;
  if (str.toLower().indexOf("poweroff") != -1)
    return MachineState_PoweredOff;
  if (str.toLower().indexOf("saved") != -1)
    return MachineState_Saved;
  if (str.toLower().indexOf("teleported") != -1)
    return MachineState_Teleported;
  if (str.toLower().indexOf("aborted") != -1)
    return MachineState_Aborted;
  if (str.toLower().indexOf("running") != -1)
    return MachineState_Running;
  if (str.toLower().indexOf("paused") != -1)
    return MachineState_Paused;
  if (str.toLower().indexOf("stuck") != -1)
    return MachineState_Stuck;
  if (str.toLower().indexOf("teleporting") != -1)
    return MachineState_Teleporting;
  if (str.toLower().indexOf("live snapshotting") != -1)
    return MachineState_LiveSnapshotting;
  if (str.toLower().indexOf("starting") != -1)
    return MachineState_Starting;
  if (str.toLower().indexOf("stopping") != -1)
    return MachineState_Stopping;
  if (str.toLower().indexOf("saving") != -1)
    return MachineState_Saving;
  if (str.toLower().indexOf("restoring") != -1)
    return MachineState_Restoring;
  if (str.toLower().indexOf("teleporting paused vm") != -1)
    return MachineState_TeleportingPausedVM;
  if (str.toLower().indexOf("teleporting in")  != -1)
    return MachineState_TeleportingIn;
  if (str.toLower().indexOf("fault tolerant syncing") != -1)
    return MachineState_FaultTolerantSyncing;
  if (str.toLower().indexOf("deleting snapshot online") != -1)
    return MachineState_DeletingSnapshotOnline;
  if (str.toLower().indexOf("deleting snapshot paused") != -1)
    return MachineState_DeletingSnapshotPaused;
  if (str.toLower().indexOf("online snapshotting") != -1)
    return MachineState_OnlineSnapshotting;
  if (str.toLower().indexOf("restoring snapshot") != -1)
    return MachineState_RestoringSnapshot;
  if (str.toLower().indexOf("deleting snapshot") != -1)
    return MachineState_DeletingSnapshot;
  if (str.toLower().indexOf("setting up") != -1)
    return MachineState_SettingUp;
  return MachineState_Null;
}
////////////////////////////////////////////////////////////////////////////
