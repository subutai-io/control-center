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
  if (str.toLower() == "null") return MachineState_Null;
  if (str.toLower() == "poweroff") return MachineState_PoweredOff;
  if (str.toLower() == "saved") return MachineState_Saved;
  if (str.toLower() == "teleported") return MachineState_Teleported;
  if (str.toLower() == "aborted") return MachineState_Aborted;
  if (str.toLower() == "running") return MachineState_Running;
  if (str.toLower() == "paused") return MachineState_Paused;
  if (str.toLower() == "stuck") return MachineState_Stuck;
  if (str.toLower() == "teleporting") return MachineState_Teleporting;
  if (str.toLower() == "live snapshotting") return MachineState_LiveSnapshotting;
  if (str.toLower() == "starting") return MachineState_Starting;
  if (str.toLower() == "stopping") return MachineState_Stopping;
  if (str.toLower() == "saving") return MachineState_Saving;
  if (str.toLower() == "restoring") return MachineState_Restoring;
  if (str.toLower() == "teleporting paused vm") return MachineState_TeleportingPausedVM;
  if (str.toLower() == "teleporting in") return MachineState_TeleportingIn;
  if (str.toLower() == "fault tolerant syncing") return MachineState_FaultTolerantSyncing;
  if (str.toLower() == "deleting snapshot online") return MachineState_DeletingSnapshotOnline;
  if (str.toLower() == "deleting snapshot paused") return MachineState_DeletingSnapshotPaused;
  if (str.toLower() == "online snapshotting") return MachineState_OnlineSnapshotting;
  if (str.toLower() == "restoring snapshot") return MachineState_RestoringSnapshot;
  if (str.toLower() == "deleting snapshot") return MachineState_DeletingSnapshot;
  if (str.toLower() == "setting up") return MachineState_SettingUp;
  return MachineState_Null;
}
////////////////////////////////////////////////////////////////////////////
