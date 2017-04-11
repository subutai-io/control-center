#include <QRegExp>

#include "VBoxManager.h"
#include "SystemCallWrapper.h"
#include "OsBranchConsts.h"
#include "Commons.h"
#include "SettingsManager.h"
#include "ApplicationLog.h"

static const int VBOXMANAGE_TIMEOUT = 5000;

CVboxManager::CVboxManager() {
  connect(&m_refresh_timer, SIGNAL(timeout()), this, SLOT(refresh_timer_timeout()));
  m_refresh_timer.setInterval(VBOXMANAGE_TIMEOUT);
  m_refresh_timer.start();

  int exit_code;
  QStringList args, out;

  args << "-v";
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, true, VBOXMANAGE_TIMEOUT);

  if (st_res == SCWE_SUCCESS && !out.empty()) {
    m_version = out[0];
  }
}

CVboxManager::~CVboxManager() {
  for (auto item : m_dct_machines)
    if (item.second) delete item.second;
}
////////////////////////////////////////////////////////////////////////////

void
CVboxManager::update_machine_state(const QString &vm_id) {
  int exit_code;
  QStringList args, out;
  args << "showvminfo" << vm_id << "--machinereadable";

  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, true, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("update_machine_state failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return;
  }
  CApplicationLog::Instance()->LogTrace("update_machine_state exit code : %d", exit_code);

  for (QString item : out) {
    int index;
    if ((index = item.indexOf("VMState=")) == -1) continue;
    QString state_str = item.mid(index+9, item.length()-index-9-1); //len(VMState=)+1. and without last symbol
    MachineState_T ns = CVirtualMachine::vm_state_from_str(state_str);
    MachineState_T os = m_dct_machines[vm_id]->state();
    m_dct_machines[vm_id]->set_machine_state(ns);
    if (ns != os) emit vm_state_changed(vm_id);
  }
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::init_machines() {
  int exit_code;
  QStringList args, out;
  args << "list" << "vms";
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, true, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("init_machines failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return st_res;
  }
  CApplicationLog::Instance()->LogTrace("init_machines exit code : %d", exit_code);

  QRegExp part0("\".+\"");
  QRegExp part1("\\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\}");

  for (QString item : out) {
    int i0, i1, l0, l1;
    i0 = part0.indexIn(item);
    i1 = part1.indexIn(item);
    l0 = part0.matchedLength();
    l1 = part1.matchedLength();
    if (i0 == -1 || i1 == -1) continue;

    QString name = item.mid(i0, l0);
    QString id = item.mid(i1, l1);

    if (name.indexOf("subutai") == -1) continue;
    if (m_dct_machines.find(id) != m_dct_machines.end()) continue;

    CVirtualMachine* machine = new CVirtualMachine(id, name);
    m_dct_machines[id] = machine;
    update_machine_state(id);
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////

CVirtualMachine *
CVboxManager::vm_by_id(const QString &vm_id) {
  auto iter = m_dct_machines.find(vm_id);
  return iter == m_dct_machines.end() ? NULL : iter->second;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::launch_vm(const QString &vm_id) {
  int exit_code;
  QStringList args, out;

  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return -1;
  args << "startvm" << vm_id << "--type" << "headless";
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, false, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("launch_vm failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return st_res;
  }

  CApplicationLog::Instance()->LogTrace("launch_vm exit code : %d", exit_code);
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::pause(const QString &vm_id) {
  int exit_code;
  QStringList args, out;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return -1;

  args << "controlvm" << vm_id << "pause";
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, false, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("pause failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return st_res;
  }
  CApplicationLog::Instance()->LogTrace("pause exit code : %d", exit_code);
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::resume(const QString &vm_id) {
  int exit_code;
  QStringList args, out;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return -1;

  args << "controlvm" << vm_id << "resume" ;
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, false, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("resume failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return st_res;
  }
  CApplicationLog::Instance()->LogTrace("resume exit code : %d", exit_code);
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::poweroff(const QString &vm_id) {
  int exit_code;
  QStringList args, out;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return -1;

  args << "controlvm" << vm_id << "poweroff" ;
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, false, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    CApplicationLog::Instance()->LogTrace("poweroff failed with err : %s",
                                          CSystemCallWrapper::scwe_error_to_str(st_res).toStdString().c_str());
    return st_res;
  }
  CApplicationLog::Instance()->LogTrace("powerofff exit code : %d", exit_code);
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::add(const QString &vm_id) {
  UNUSED_ARG(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t
CVboxManager::remove(const QString &vm_id) {
  int exit_code;
  QStringList args, out;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end())
    return -1;

  args << "unregistervm" << vm_id;
  system_call_wrapper_error_t st_res =
      CSystemCallWrapper::ssystem_th(CSettingsManager::Instance().vboxmanage_path(),
                                     args, out, exit_code, false, VBOXMANAGE_TIMEOUT);

  if (st_res != SCWE_SUCCESS) {
    //todo log/notify
    return st_res;
  }
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void
CVboxManager::refresh_timer_timeout() {
  m_refresh_timer.stop();
  init_machines();
  for (auto pair : m_dct_machines) {
    update_machine_state(pair.first);
  }
  m_refresh_timer.start();
}
////////////////////////////////////////////////////////////////////////////
