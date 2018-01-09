#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H
#include <QObject>

enum MachineState_T {
  MachineState_Null = 0                ,
  MachineState_PoweredOff              ,
  MachineState_Saved                   ,
  MachineState_Teleported              ,
  MachineState_Aborted                 ,
  MachineState_Running                 ,
  MachineState_Paused                  ,
  MachineState_Stuck                   ,
  MachineState_Teleporting             ,
  MachineState_LiveSnapshotting        ,
  MachineState_Starting                ,
  MachineState_Stopping                ,
  MachineState_Saving                  ,
  MachineState_Restoring               ,
  MachineState_TeleportingPausedVM     ,
  MachineState_TeleportingIn           ,
  MachineState_FaultTolerantSyncing    ,
  MachineState_DeletingSnapshotOnline  ,
  MachineState_DeletingSnapshotPaused  ,
  MachineState_OnlineSnapshotting      ,
  MachineState_RestoringSnapshot       ,
  MachineState_DeletingSnapshot        ,
  MachineState_SettingUp
};

/**
 * @brief The CVirtualMachine represents Virtual Box VM
 */
class CVirtualMachine : public QObject {
  Q_OBJECT
private:
  QString m_id;
  QString m_name;
  MachineState_T m_state;
public:
  CVirtualMachine(const QString& id,
                  const QString& name);
  virtual ~CVirtualMachine();

  static const char* vm_state_to_str(MachineState_T state);
  static MachineState_T vm_state_from_str(const QString& str);

  void set_machine_state(MachineState_T ns) {m_state = ns;}
  MachineState_T state() const {return m_state;}
  const QString& id() const {return m_id;}
  const QString& name() const {return m_name;}

private slots:
public slots:
signals:
};

#endif // VIRTUALMACHINE_H
