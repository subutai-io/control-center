#include <QRegExp>

#include "Commons.h"
#include "OsBranchConsts.h"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "VBoxManager.h"

static const int VBOXMANAGE_TIMEOUT = 5000;

CVboxManager::CVboxManager(QObject *parent) :
  QObject(parent),
  m_version(tr("Couldn't get VBox version, sorry")),
  m_refresh_timer(nullptr) {}

CVboxManager::~CVboxManager() {
  for (auto item : m_dct_machines)
    if (item.second) delete item.second;
}
////////////////////////////////////////////////////////////////////////////

void CVboxManager::update_machine_state(const QString &vm_id) {
  QStringList args;
  args << "showvminfo" << vm_id << "--machinereadable";

  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, true, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return;
  }

  for (QString item : st_res.out) {
    int index;
    if ((index = item.indexOf("VMState=")) == -1) continue;
    // len(VMState=)+1. and without last symbol
    QString state_str = item.mid(index + 9, item.length() - index - 9 - 1);
    MachineState_T ns = CVirtualMachine::vm_state_from_str(state_str);
    MachineState_T os = m_dct_machines[vm_id]->state();
    m_dct_machines[vm_id]->set_machine_state(ns);
    if (ns != os) emit vm_state_changed(vm_id);
  }
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::init_machines() {
  m_refresh_timer->stop();
  QStringList args;
  args << "list"
       << "vms";
  system_call_res_t st_res = CSystemCallWrapper::ssystem(
      CSettingsManager::Instance().vboxmanage_path(), args, true, false);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    m_refresh_timer->start();
    return st_res.res;
  }

  QRegExp part0("\".+\"");
  QRegExp part1(
      "\\{[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-"
      "F]{12}\\}");

  std::vector<QString> lst_existing_vms;

  for (QString item : st_res.out) {
    int i0, i1, l0, l1;
    i0 = part0.indexIn(item);
    i1 = part1.indexIn(item);
    l0 = part0.matchedLength();
    l1 = part1.matchedLength();
    if (i0 == -1 || i1 == -1) continue;

    QString name = item.mid(i0, l0);
    QString id = item.mid(i1, l1);

    if (name.indexOf("subutai") == -1) continue;

    lst_existing_vms.push_back(id);
    if (m_dct_machines.find(id) != m_dct_machines.end()) continue;

    CVirtualMachine *machine = new CVirtualMachine(id, name);
    m_dct_machines[id] = machine;
    emit vm_add(machine->id());
    update_machine_state(id);
  }

  std::vector<QString> lst_vm_to_remove;
  for (auto vm = m_dct_machines.begin(); vm != m_dct_machines.end(); ++vm) {
    QString id = vm->first;
    if (std::find(lst_existing_vms.begin(), lst_existing_vms.end(), id) ==
        lst_existing_vms.end()) {
      lst_vm_to_remove.push_back(id);
    }
  }

  for (auto vm_id : lst_vm_to_remove) {
    m_dct_machines.erase(vm_id);
    emit vm_remove(vm_id);
  }

  m_refresh_timer->start();
  return 0;
}
////////////////////////////////////////////////////////////////////////////

CVirtualMachine *CVboxManager::vm_by_id(const QString &vm_id) {
  auto iter = m_dct_machines.find(vm_id);
  return iter == m_dct_machines.end() ? NULL : iter->second;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::launch_vm(const QString &vm_id) {
  QStringList args;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end()) return -1;

  args << "startvm" << vm_id << "--type"
       << "headless";
  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, false, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return st_res.res;
  }

  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::pause(const QString &vm_id) {
  QStringList args;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end()) return -1;

  args << "controlvm" << vm_id << "pause";
  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, false, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return st_res.res;
  }

  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::resume(const QString &vm_id) {
  QStringList args;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end()) return -1;

  args << "controlvm" << vm_id << "resume";
  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, false, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return st_res.res;
  }

  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::poweroff(const QString &vm_id) {
  QStringList args;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end()) return -1;

  args << "controlvm" << vm_id << "poweroff";
  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, false, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return st_res.res;
  }

  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::add(const QString &vm_id) {
  UNUSED_ARG(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int32_t CVboxManager::remove(const QString &vm_id) {
  QStringList args;
  if (m_dct_machines.find(vm_id) == m_dct_machines.end()) return -1;

  args << "unregistervm" << vm_id;
  system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vboxmanage_path(), args, false, false,
      VBOXMANAGE_TIMEOUT);

  if (st_res.res != SCWE_SUCCESS || st_res.exit_code != 0) {
    return st_res.res;
  }
  update_machine_state(vm_id);
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void CVboxManager::refresh_timer_timeout() {
  m_refresh_timer->stop();
  init_machines();
  for (auto pair : m_dct_machines) {
    update_machine_state(pair.first);
  }
  m_refresh_timer->start();
}
////////////////////////////////////////////////////////////////////////////

void CVboxManager::start_work() {
  try {
    m_refresh_timer = new QTimer(this);
    connect(m_refresh_timer, &QTimer::timeout, this,
            &CVboxManager::refresh_timer_timeout);
    m_refresh_timer->setInterval(VBOXMANAGE_TIMEOUT + 1000);
    m_refresh_timer->start();

    init_machines();
    QStringList args;
    args << "-v";
    system_call_res_t st_res = CSystemCallWrapper::ssystem_th(
        CSettingsManager::Instance().vboxmanage_path(), args, true, false,
        VBOXMANAGE_TIMEOUT);

    if (st_res.res == SCWE_SUCCESS && !st_res.out.empty()) {
      m_version = st_res.out[0];
    }
  } catch (std::exception &exc) {
    qCritical("CVboxManager::start_work() exc : %s",
                                          exc.what());
  }

  emit initialized();
}
////////////////////////////////////////////////////////////////////////////
