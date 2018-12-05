#include <stdio.h>
#include <sstream>

#include <SystemCallWrapper.h>
#include <QApplication>
#include <QDesktopServices>
#include <QHostAddress>
#include <QCoreApplication>
#include <QNetworkInterface>
#include <QProcess>
#include <QSysInfo>
#include <QUrl>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>
#include <QMessageBox>
#include <QFile>

#include "HubController.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "RestWorker.h"
#include "SettingsManager.h"
#include "LibsshController.h"
#include "X2GoClient.h"
#include "VagrantProvider.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <thread>
#include <chrono>

#ifdef RT_OS_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#pragma comment(lib, "Advapi32.lib")
#endif


struct proc_controller{
    int proc_hash;
    QProcess *proc;
    proc_controller(QProcess &proc){
        proc_hash = CProcessHandler::Instance()->start_proc(proc);
    }
    ~proc_controller(){
        CProcessHandler::Instance()->end_proc(proc_hash);
    }
};

static QString error_strings[] = {"Success",
                                  "Shell error",
                                  "Pipe error",
                                  "Set handle info error",
                                  "Create process error",
                                  "Cant join to swarm",
                                  "SSH launch error",
                                  "System call timeout"};

system_call_res_t CSystemCallWrapper::ssystem_th(const QString &cmd,
                                                 QStringList &args,
                                                 bool read_output, bool log,
                                                 unsigned long timeout_msec) {
  QFuture<system_call_res_t> f1 =
      QtConcurrent::run(ssystem, cmd, args, read_output, log, timeout_msec);
  f1.waitForFinished();
  return f1.result();
}
////////////////////////////////////////////////////////////////////////////
/// \brief CSystemCallWrapper::ssystem_f
/// \param cmd
/// \param args
/// \param read_out
/// \param log
/// \param timeout_msec
/// \brief Writes STDERR to "ouput" file in temp folder.
/// \brief Returns system ouput from "output" file in temp folder.
/// \return
///
system_call_res_t CSystemCallWrapper::ssystem_f(QString cmd, QStringList args,
                                                bool read_out, bool log,
                                                unsigned long timeout_msec) {
  QProcess proc;
  if(args.begin() != args.end() && args.size() >= 2){
      if(*(args.begin())=="set_working_directory"){
          args.erase(args.begin());
          proc.setWorkingDirectory(*(args.begin()));
          args.erase(args.begin());
      }
  }
  system_call_res_t res = {SCWE_SUCCESS, QStringList(), 0};

  // Get temp folder path
  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

  if (lst_temp.empty()) {
    QString err_msg = QObject::tr("Unable to get the standard temporary location. "
                                  "Verify that your file system is setup correctly and fix any issues.");
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    res.res = SCWE_WRONG_TEMP_PATH;
    return res;
  }

  QString tmpFilePath =
      lst_temp[0] + QDir::separator() + "output";

  QFile tmpFile(tmpFilePath);

  QStringList tmp;
  tmp = args;
  args.clear();

  switch (CURRENT_OS) {
  case OS_LINUX:
    args << "-c"
         << QString("%1 %2 2> %3").arg(cmd, tmp.join(" "), tmpFilePath);
    cmd = QString("sh");

    break;
  case OS_MAC:
    args << "-e"
         << QString("do shell script \"%1 %2 2> %3\"").arg(cmd, tmp.join(" "), tmpFilePath);
    cmd = "osascript";

    break;
  case OS_WIN:
    args << "-NoLogo"
         << "-NoProfile"
         << "-NonInteractive"
         << "-ExecutionPolicy"
         << "Bypass"
         << "-Command"
         << QString("%1 %2 > \"%3\" 2>&1").arg(cmd, tmp.join(" "), tmpFilePath);
    cmd = "powershell";
    break;
  default:
    break;
  }

  qDebug() << "ssystem_f: "
           << "cmd: "
           << cmd
           << "args: "
           << args;

  proc.start(cmd, args);
  proc_controller started_proc(proc);
  if(timeout_msec == 97) {

      if (!proc.waitForStarted(-1)) {
        if (log) {
          qCritical(
              "Failed to wait for started process %s", cmd.toStdString().c_str());
          qCritical(
              "%s", proc.errorString().toStdString().c_str());
        }
        res.res = SCWE_CREATE_PROCESS;
        return res;
      }

      if (!proc.waitForFinished(-1)) {
        proc.terminate();
        res.res = SCWE_TIMEOUT;
        return res;
      }
  } else {
      if (!proc.waitForStarted(timeout_msec)) {
        if (log) {
          qCritical(
              "Failed to wait for started process %s", cmd.toStdString().c_str());
          qCritical(
              "%s", proc.errorString().toStdString().c_str());
        }
        res.res = SCWE_CREATE_PROCESS;
        return res;
      }

      if (!proc.waitForFinished(timeout_msec)) {
        proc.terminate();
        res.res = SCWE_TIMEOUT;
        return res;
      }
  }

  if (read_out) {
    QFileInfo info(tmpFilePath);
    if (info.exists()) {
      if (tmpFile.open(QIODevice::ReadWrite) ) {
        QTextStream stream(&tmpFile);
        QString output = QString(stream.readAll());
        res.out = output.split("\n", QString::SkipEmptyParts);
      } else {
        res.res = SCWE_WRONG_FILE_NAME;
        return res;
      }
    }
  }

  res.exit_code = proc.exitCode();
  res.res = proc.exitStatus() == QProcess::NormalExit ? SCWE_SUCCESS
                                                      : SCWE_PROCESS_CRASHED;
  return res;
}
////////////////////////////////////////////////////////////////////////////
system_call_res_t CSystemCallWrapper::ssystem(const QString &cmd,
                                              QStringList &args,
                                              bool read_out, bool log,
                                              unsigned long timeout_msec) {
  QProcess proc;
  if(args.begin() != args.end() && args.size() >= 2){
      if(*(args.begin())=="set_working_directory"){
          args.erase(args.begin());
          proc.setWorkingDirectory(*(args.begin()));
          args.erase(args.begin());
      }
  }
  system_call_res_t res = {SCWE_SUCCESS, QStringList(), 0};

#ifdef RT_OS_DARWIN
  // Vagrant can't find VBoxManage binary path while checking peer status by vagrant status
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PATH", env.value("PATH") + ":/usr/local/bin");
  proc.setProcessEnvironment(env);
#endif

  proc.start(cmd, args);

  proc_controller started_proc(proc);
  if(timeout_msec == 97) {

      if (!proc.waitForStarted(-1)) {
        if (log) {
          qCritical(
              "Failed to wait for started process %s", cmd.toStdString().c_str());
          qCritical(
              "%s", proc.errorString().toStdString().c_str());
        }
        res.res = SCWE_CREATE_PROCESS;
        return res;
      }

      if (!proc.waitForFinished(-1)) {
        proc.terminate();
        res.res = SCWE_TIMEOUT;
        return res;
      }
  }
  else{

      if (!proc.waitForStarted(timeout_msec)) {
        if (log) {
          qCritical(
              "Failed to wait for started process %s", cmd.toStdString().c_str());
          qCritical(
              "%s", proc.errorString().toStdString().c_str());
        }
        res.res = SCWE_CREATE_PROCESS;
        return res;
      }

      if (!proc.waitForFinished(timeout_msec)) {
        proc.terminate();
        res.res = SCWE_TIMEOUT;
        return res;
      }
  }
  if (read_out) {
    QString output = QString(proc.readAll());
    res.out = output.split("\n", QString::SkipEmptyParts);
  }

  res.exit_code = proc.exitCode();
  res.res = proc.exitStatus() == QProcess::NormalExit ? SCWE_SUCCESS
                                                      : SCWE_PROCESS_CRASHED;
  return res;
}
////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::is_in_swarm(const QString &hash) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show"; // need to change
  system_call_res_t res = ssystem_th(cmd, args, true, true);

  // qInfo("is_in_swarm %s show %s", cmd.toStdString().c_str(), hash.toStdString().c_str());

  if (res.res != SCWE_SUCCESS && res.exit_code != 1) {
    CNotificationObserver::Error(QObject::tr((error_strings[res.res]).toStdString().c_str()), DlgNotification::N_NO_ACTION);
    qCritical("%s", error_strings[res.res].toStdString().c_str());
    return false;
  }

  // todo use indexOf
  bool is_in = false;
  for (auto i = res.out.begin(); i != res.out.end() && !is_in; ++i) {
    is_in |= (i->indexOf(hash) != -1);
  }
  return is_in;
}
////////////////////////////////////////////////////////////////////////////

std::vector<std::pair<QString, QString>> CSystemCallWrapper::p2p_show_interfaces() {
  std::vector<std::pair<QString, QString>> swarm_lsts;

  if (!p2p_daemon_check()) {
    return swarm_lsts;
  }

  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;


  args << "show" << "--interfaces" << "--bind";
  system_call_res_t res = ssystem_th(cmd, args, true, true);

  if (res.res != SCWE_SUCCESS && res.exit_code != 1) {
    qCritical("%s", error_strings[res.res].toStdString().c_str());
    return swarm_lsts;
  }

  for (QString swarm : res.out) {
    if (swarm.indexOf("swarm") != -1) {
      QStringList lst = swarm.split("|", QString::SkipEmptyParts);
      if (lst.empty() || lst.size() != 2) {
        continue;
      }
      swarm_lsts.push_back(std::make_pair(lst[0], lst[1]));
    }
  }

  return swarm_lsts;
}

////////////////////////////////////////////////////////////////////////////

std::vector<QString> CSystemCallWrapper::p2p_show() {
  std::vector<QString> swarm_lsts;

  if (!p2p_daemon_check()) {
    return swarm_lsts;
  }

  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;


  args << "show";
  system_call_res_t res = ssystem_th(cmd, args, true, true);

  if (res.res != SCWE_SUCCESS && res.exit_code != 1) {
    qCritical("%s", error_strings[res.res].toStdString().c_str());
    return swarm_lsts;
  }

  for (auto i = res.out.begin(); i != res.out.end(); ++i) {
    if (i->indexOf("swarm") != -1) {
      swarm_lsts.push_back(i->mid(i->indexOf("swarm"), i->length()));
    }
  }

  return swarm_lsts;
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
std::pair<system_call_wrapper_error_t, QStringList> remove_file_internal(const QString &file_path);

template<>
std::pair<system_call_wrapper_error_t, QStringList> remove_file_internal<Os2Type<OS_LINUX>>(const QString &file_path) {
  QString cmd("rm");
  QStringList args;
  args << "-rf" << file_path;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  qDebug() << "Finished remove command:"
           << "cmd:" << cmd
           << "args:" << args;

  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to remove file:" << file_path
                << "exit code:" << res.exit_code
                << "output:" << res.out;
  }
  return std::make_pair(res.res, res.out);
}

template<>
std::pair<system_call_wrapper_error_t, QStringList> remove_file_internal<Os2Type<OS_MAC>>(const QString &file_path) {
  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << QString("do shell script \"rm -rf %1\"").arg(file_path);

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  qDebug() << "Finished remove command:"
           << "cmd:" << cmd
           << "args:" << args;

  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to remove file:" << file_path
                << "exit code:" << res.exit_code
                << "output:" << res.out;
  }
  return std::make_pair(res.res, res.out);
}

template<>
std::pair<system_call_wrapper_error_t, QStringList> remove_file_internal<Os2Type<OS_WIN>>(const QString &file_path) {
  QFileInfo file_info(file_path);
  if (!file_info.exists()) {
    qCritical() << "File doesn't exist or wrong file name" << file_path;
    return std::make_pair(SCWE_WRONG_FILE_NAME, QStringList());
  }

  if (!file_info.isWritable()) {
    qCritical() << "File isn't editable" << file_path;
    return std::make_pair(SCWE_PERMISSION_DENIED, QStringList());
  }

  if (file_info.isDir()) {
    QDir dir(file_path);
    if (!dir.removeRecursively()) {
      qCritical() << "Failed to remove directory:" << file_path;
      return std::make_pair(SCWE_COMMAND_FAILED, QStringList());
    }
  } else {
    if (!QFile::remove(file_path)) {
      qCritical() << "Failed to remove file:" << file_path;
      return std::make_pair(SCWE_COMMAND_FAILED, QStringList());
    }
  }

  return std::make_pair(SCWE_SUCCESS, QStringList());
}

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::remove_file(const QString &file_path) {
  return remove_file_internal<Os2Type<CURRENT_OS>>(file_path);
}

////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::create_folder(const QString &dir, const QString &name) {
  qDebug() << "started create_folder with args:" << dir << name;

  QDir current_dir(dir);
  if (!current_dir.exists()) {
    qCritical() << dir << "doesn't exist";
    return SCWE_DIR_DOESNT_EXIST;
  }

  if (current_dir.cd(name) || current_dir.exists(name)) {
    qCritical() << "'" << name << "' in '" << dir << "' already exists";
    return SCWE_DIR_EXISTS;
  }

  if (!current_dir.mkdir(name)) {
    qCritical() << "Can't create" << name << "in" << dir << "permission denied";
    return SCWE_PERMISSION_DENIED;
  }

  qDebug() << "Successfully created folder" << name << "in" << dir;
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::send_command(
    const QString &remote_user, const QString &ip, const QString &port,
    const QString &commands, const QString &key) {

  QString cmd
      = QString("%1").arg(CSettingsManager::Instance().ssh_path());
  QStringList args;
  if (port.isEmpty()) {
    args
         << "-o" << "StrictHostKeyChecking=no"
         << QString("%1@%2").arg(remote_user, ip)
         << "-i" << QString("%1").arg(key)
         << QString("%1").arg(commands);
  } else {
    args
         << "-o" << "StrictHostKeyChecking=no"
         << QString("%1@%2").arg(remote_user, ip)
         << "-p" << port
         << "-i" << QString("%1").arg(key)
         << QString("%1").arg(commands);
  }
  qDebug() << "Transfer file remote command ARGS=" << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 10000);
  qDebug() << "Transfer file remote command" << args
           << "finished"
           << "exit code:" <<res.exit_code
           << "res" << res.res
           << "output" << res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code != 0) {
    return std::make_pair(SCWE_CREATE_PROCESS, res.out);
  }
  return std::make_pair(res.res, res.out);
}


std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::upload_file
(const QString &remote_user, const QString &ip, std::pair<QString, QString> ssh_info,
 const QString &destination, const QString &file_path) {
  QString cmd
      = CSettingsManager::Instance().scp_path();
  QStringList args;

  QString destination_formatted = destination.contains(" ") ?
        QString("\"%1\"").arg(destination) : destination;

  args<< "-rp"
      << "-o StrictHostKeyChecking=no"
      << "-S" << CSettingsManager::Instance().ssh_path()
      << "-i" << ssh_info.second;

  if (!ssh_info.first.isEmpty())
    args << "-P" << ssh_info.first;

  args
      << file_path
      << QString("%1@%2:%3").arg(remote_user, ip, destination_formatted);
  qDebug() << "ARGS=" << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);
  qDebug() << file_path << "finished:"
           << "exit code:" <<res.exit_code
           << "result code:" <<res.res
           << "output: " <<res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code != 0) {
    return std::make_pair(SCWE_CREATE_PROCESS, res.out);
  }
  return std::make_pair(res.res, res.out);
}

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::download_file
(const QString &remote_user, const QString &ip, std::pair<QString, QString> ssh_info,
 const QString &local_destination, const QString &remote_file_path) {
  QString cmd
      = CSettingsManager::Instance().scp_path();
  QStringList args;

  QString remote_file_path_formatted = remote_file_path.contains(" ") ?
        QString("\"%1\"").arg(remote_file_path) : remote_file_path;

  args << "-rp"
       << "-o StrictHostKeyChecking=no"
       << "-S" << CSettingsManager::Instance().ssh_path()
       << "-i" << ssh_info.second;

  if (!ssh_info.first.isEmpty())
    args << "-P" << ssh_info.first;

  args
       << QString("%1@%2:%3").arg(remote_user, ip, remote_file_path_formatted)
       << local_destination;
  qDebug() << "ARGS=" << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);
  if (res.res == SCWE_SUCCESS && res.exit_code != 0) {
    return std::make_pair(SCWE_CREATE_PROCESS, res.out);
  }
  return std::make_pair(res.res, res.out);
}

//////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::vagrant_init(const QString &dir, const QString &box){
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args
        << "set_working_directory"
        << dir
        << "init";
    if (box == "Debian Stretch")
        CURRENT_BRANCH == BT_PROD ? args << "subutai/stretch" : args << "subutai/stretch-master";
    else args << "subutai/xenial";


    qDebug()
            <<"Vagrant init ARGS: "
            <<args;

    system_call_res_t res = ssystem_th(cmd, args, true, true, 10000);

    qDebug()
            <<"Finished vagrant init:"
            <<args
            <<res.res;

    if(res.res == SCWE_SUCCESS && res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    return res.res;
}

// parse into args line of vagrant global-status
// algo: just seperate string into 5 and parse
void parse_status_line(QString status_line,
                       int index_state,
                       int index_directory,
                       QString &id,
                       QString &name,
                       QString &provider,
                       QString &state,
                       QString &directory) {
  state = "";
  QStringList seperated = status_line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  if(seperated.size() < 5) return;
  QString *args[] {&id, &name, &provider};

  for (size_t i = 0; i < 3; i++){
      *args[i] = seperated[0];
      seperated.erase(seperated.begin());
  }

  int size = seperated.size();

  if (size > 2) {
    state = status_line.mid(index_state, index_directory - index_state).trimmed();
    directory = status_line.mid(index_directory, status_line.size() - index_directory).trimmed();
    return;
  } else {
    state = seperated[0];
    seperated.erase(seperated.begin());
  }

  if (!seperated.empty()) {
    directory = status_line.remove(0, status_line.indexOf(seperated[0]));
    while (!directory[directory.size() -1].isLetterOrNumber() && !directory.isEmpty()) {
      directory.remove(directory.size() -1 , 1);
    }
    return;
  }
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_update_peeros(const QString &port, const QString &peer_name){
  if (CRestWorker::Instance()->peer_update_management(port)) {
    CNotificationObserver::Instance()->Info(QObject::tr("Successfully finished to update Peer %2")
                                            .arg(peer_name), DlgNotification::N_NO_ACTION);
  } else {
    CNotificationObserver::Instance()->Error(QObject::tr("Failed to update Peer %2 with. Please make sure your Peer is running "
                                                "and try to update from Subutai Console")
                                             .arg(peer_name), DlgNotification::N_NO_ACTION);
    return SCWE_CREATE_PROCESS;
  }
  return SCWE_SUCCESS;
}

QString CSystemCallWrapper::vagrant_status(const QString &dir) {
  QMutexLocker locker(&vagrant_is_busy);
  qDebug() << "get vagrant status of" << dir;

  system_call_res_t res;
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  QString status("not_created");

  args << "set_working_directory"
       << dir
       << "status"
       << "--machine-readable";

  res = ssystem_th(cmd, args, true, true, 50000);

  qDebug() << "Vagrant status result: "
           << res.res
           << res.exit_code
           << res.out
           << "args: "
           << args;

  if(res.res != SCWE_SUCCESS || res.exit_code != 0) {
    return status;
  }

  for (auto s : res.out) {
    QStringList seperated = s.split(",");
    if (seperated.contains("state")) {
      status = seperated.takeLast();
      qDebug() << "FOUND vagrant status:"
               << status.simplified();
      break;
    }
  }

  return status.simplified();
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_halt(const QString &dir) {
  QMutexLocker locker(&vagrant_is_busy);
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args << "set_working_directory"
       << dir
       << "halt";

  qDebug() << "Starting to halt peer. Args:"
           << args;
  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);

  qDebug() << "Halt finished:"
           << dir
           << res.res;

  if(res.res == SCWE_SUCCESS && res.exit_code != 0) {
    return SCWE_CREATE_PROCESS;
  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_reload(const QString &dir) {
  QMutexLocker locker(&vagrant_is_busy);
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args
      << "set_working_directory"
      << dir
      << "reload";

  qDebug()
          <<"Starting to reload peer. Args:"
          <<args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);

  qDebug()
          <<"Reload finished:"
          <<dir
          <<res.res;

  if(res.res == SCWE_SUCCESS && res.exit_code != 0) {
    return SCWE_CREATE_PROCESS;
  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_destroy(const QString &dir) {
  QMutexLocker locker(&vagrant_is_busy);
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args << "set_working_directory"
       << dir
       << "destroy"
       << "-f";

  qDebug() << "Starting to destroy peer. Args:"
           << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);

  qDebug() << "Destroying peer finished"
           << "Exit code:"
           << res.exit_code
           << "Result:"
           << res.res
           << "Output:"
           << res.out;

  if(res.exit_code !=0 || res.res != SCWE_SUCCESS) {
    return SCWE_CREATE_PROCESS;
  }

  QDir dir_path(dir);

  if(dir_path.removeRecursively()) {
    return SCWE_SUCCESS;
  }

  return SCWE_CREATE_PROCESS;
}

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::vagrant_up(const QString &dir) {
  QMutexLocker locker(&vagrant_is_busy);
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args
      << "set_working_directory"
      << dir
      << "up";

  qDebug()
          <<"Vagrant up. Args:"
          <<args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 97);

  qDebug()
          <<"Finished vagrant up:"
          <<dir
          <<"exit code:"<<res.exit_code
          <<"result code:"<<res.res;

  if(res.res == SCWE_SUCCESS && res.exit_code != 0)
      res.res = SCWE_CREATE_PROCESS;

  return std::make_pair(res.res, res.out);
}

QString CSystemCallWrapper::vagrant_ip(const QString &dir) {
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args
        << "set_working_directory"
        << dir
        << "subutai"
        << "info ipaddr";

    qDebug()
            <<"Getting vagrant ip:"
            <<args;
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 20000);
    qDebug()
            <<"Finished vagrant ip:"
            <<dir
            <<"exit code:"<<res.exit_code
            <<"result code:"<<res.res
            <<"output"<<res.out;

    QString ip = "";

    if(res.res != SCWE_SUCCESS || res.exit_code != 0 || res.out.size() != 1)
        return ip;
    ip = res.out[0];
    return ip;
}

QString CSystemCallWrapper::get_virtualbox_vm_storage(){
  // vboxmanage list systemproperties | grep folder
  qDebug("Get virtualbox vm storage");
  QStringList home_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QString vm_dir;
  if (home_path.isEmpty()) {
    qCritical("No home path on machine!");
    return vm_dir;
  } else {
    vm_dir = home_path[0];
  }
  QString path = CSettingsManager::Instance().oracle_virtualbox_path();
  QDir dir(path);
  dir.cdUp();
  QString cmd = dir.absolutePath();
  cmd += "/VBoxManage";
  QStringList args;
  args << "list" << "systemproperties";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  if (res.res != SCWE_SUCCESS ||
      res.exit_code != 0 || res.out.isEmpty()) {
    return vm_dir;
  } else { // need to parse data
    QStringList parse_me = res.out;
    QString flag;
    QString value;
    QString last_name;
    bool reading_value = false;
    for (auto s : parse_me) {
      flag = value = "";
      reading_value = false;
      for (int i = 0; i < s.size(); i++) {
        if (s[i] == '\r') {
          continue;
        } else if (reading_value) {
          value += s[i];
        } else if (s[i] == ':') {
          flag = value;
          value = "";
        } else if (s[i] != ' ') {
            if(value == "" && !flag.isEmpty())
                reading_value = true;
            value+=s[i];
        }
      }
      if (flag == "Defaultmachinefolder") {
        return value;
      }
    }
  }
  return vm_dir;
}

system_call_wrapper_error_t CSystemCallWrapper::set_virtualbox_vm_storage(const QString &vm_dir){
  // vboxmanage setproperty machinefolder /path/to/directory/
  QString path = CSettingsManager::Instance().oracle_virtualbox_path();
  QDir dir(path);
  dir.cdUp();
  QString cmd = dir.absolutePath();
  cmd += "/VBoxManage";
  QStringList args;
  args << "setproperty" << "machinefolder" << vm_dir;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug() << "set vm folder finished"
           << "exit code" << res.exit_code
           << "result code" << res.res
           << "output" << res.out;
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

QString CSystemCallWrapper::vagrant_port(const QString &dir) {
  QString port_or_ip_address = "_CONSOLE_PORT";

  if (VagrantProvider::Instance()->UseIp()) {
    port_or_ip_address = "_IP_PEER";
  }

  QDir peer_dir(dir);
  QString  port = "undefined";

  if (peer_dir.cd(".vagrant")) {
    QFile file(QString(peer_dir.absolutePath() + QDir::separator() + "generated.yml"));
    QString file_name = file.fileName();

    if (file.exists()) {
      if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        QString output = QString(stream.readAll());
        QStringList vagrant_info = output.split("\n", QString::SkipEmptyParts);

        for (auto s : vagrant_info) {
          QString flag, value;
          bool reading_value = false;
          flag = value = "";
          reading_value = false;

          for (int i=0; i < s.size(); i++) {
            if (s[i]=='\r') continue;
            if (reading_value) {
                value += s[i];
                continue;
            }
            if (s[i] == ':') {
                flag = value;
                value = "";
                continue;
            }
            if (s[i] != ' ') {
                if(value == "" && !flag.isEmpty())
                    reading_value = true;
                value+=s[i];
            }
          }

          if (flag == port_or_ip_address)
              port = value;
          }
      }
      file.close();
    }
  }
  return port;
}

std::pair<QStringList, system_call_res_t> CSystemCallWrapper::vagrant_update_information(bool force_update) {
  QMutexLocker locker(&vagrant_is_busy);
  qDebug() << "Starting to update information related to peer management";

  QStringList bridges = CSystemCallWrapper::list_interfaces(force_update);

  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args << "global-status";

  system_call_res_t global_status = ssystem_th(cmd, args, true, true, 20000);

  qInfo() << "Vagrant global-status"
          << args
          << "exit code: "
          << global_status.exit_code
          << " res: "
          << global_status.res
          << global_status.out;

  // Clear global_status
  QStringList tmp = global_status.out;
  global_status.out.clear();

  QString p_id, p_name, p_provider, p_state, p_directory;
  int index_state, index_directory;

  if (!tmp.empty()) {
    index_state = tmp.begin()->indexOf("state");
    index_directory = tmp.begin()->indexOf("directory");
  }

  for (auto s : tmp) {
    parse_status_line(s, index_state, index_directory, p_id, p_name, p_provider,
                      p_state, p_directory);
    // ckeck is this Subutai peer
    p_directory = QDir::fromNativeSeparators(p_directory);
    QString generated_file_path = p_directory + QDir::separator() +
        ".vagrant" + QDir::separator() + "generated.yml";
    QFileInfo generated_file(generated_file_path);

    if (p_provider == VagrantProvider::Instance()->CurrentVal() && QDir(p_directory).exists() &&
        generated_file.exists()) {
      global_status.out.append(p_directory);
      qDebug() << "Vagrant found local peers: "
               << VagrantProvider::Instance()->CurrentVal()
               << " ID: "
               << p_id
               << " NAME: "
               << p_name
               << " PROVIDER: "
               << p_provider
               << " STATE: "
               << p_state
               << " DIRECTORY: "
               << p_directory;
    }
  }

  return std::make_pair(bridges, global_status);
}
//////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t give_write_permissions_internal(const QString &dir);

template<>
system_call_wrapper_error_t give_write_permissions_internal<Os2Type<OS_LINUX> >(const QString &dir) {
  UNUSED_ARG(dir);
  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t give_write_permissions_internal<Os2Type<OS_MAC> >(const QString &dir) {
  QFileInfo dir_info(dir);
  if(dir_info.isDir() && dir_info.isWritable()){
    return SCWE_SUCCESS;
  }
  QString cmd("osascript");
  QStringList args;
  args << "-e" << QString("do shell script \"chmod +w %1\" "
                          "with administrator privileges").arg(dir);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true);
  if (res.res != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("An error occured while running chmod command.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
  }
  return res.res;
}

template<>
system_call_wrapper_error_t give_write_permissions_internal<Os2Type<OS_WIN> >(const QString &dir) {
  UNUSED_ARG(dir);
  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::give_write_permissions(const QString &dir) {
  return give_write_permissions_internal<Os2Type<CURRENT_OS> >(dir);
}
//////////////////////////////////////////////////////////////////////
QStringList CSystemCallWrapper::list_interfaces(bool force_update) {
  static QStringList empty;
  static QStringList hyperv; // It gets one time when application launched.
  static QStringList virtualbox;
  static QStringList libvirt;
  static QStringList parallels;

  switch (VagrantProvider::Instance()->CurrentProvider()) {
  case VagrantProvider::VIRTUALBOX:
    if (force_update || virtualbox.empty())
      virtualbox = virtualbox_interfaces();

    return virtualbox;
  case VagrantProvider::LIBVIRT:
    if (force_update || libvirt.empty())
      libvirt = libvirt_interfaces();

    return libvirt;
  case VagrantProvider::PARALLELS:
    if (force_update || parallels.empty())
      parallels = parallels_interfaces();

    return parallels;
  case VagrantProvider::HYPERV:
    if (force_update || hyperv.empty())
      hyperv = hyperv_interfaces();

    return hyperv;
  default:
    return empty;
  }
}

QStringList CSystemCallWrapper::parallels_interfaces() {
  installer_is_busy.lock();

  qDebug("Getting list of bridged interfaces parallels");

  QStringList interfaces;
  QString cmd = "prlctl";
  QString path;

  system_call_wrapper_error_t cr;

  if ((cr = CSystemCallWrapper::which(cmd, path)) != SCWE_SUCCESS) {
    installer_is_busy.unlock();
    return interfaces;
  }

  // assign cmd full path
  cmd = path;

  QStringList args;
  args << "server"
       << "info"
       << "--json";

  qDebug() << "Parallels interface command:"
           << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 1000*3*60);

  qDebug() << "Listing parallels interfaces result:"
           << "exit code: "
           << res.exit_code
           << "result: "
           << res.res
           << "output:"
           << res.out;

  //QString output ="{\r\n    \"ID\": \"57bf3726-def4-43d4-a32f-cc0e8e357e16\",\r\n    \"Hostname\": \"127.0.0.1\",\r\n    \"Version\": \"Desktop 13.3.0-43321\",\r\n    \"OS\": \"Mac OS X 10.13.4(17E199)\",\r\n    \"Started as service\": \"off\",\r\n    \"VM home\": \"\\\/Users\\\/admin\\\/Parallels\",\r\n    \"Memory limit\": {\r\n        \"mode\": \"auto\"\r\n    },\r\n    \"Minimal security level\": \"low\",\r\n    \"Manage settings for new users\": \"allow\",\r\n    \"CEP mechanism\": \"off\",\r\n    \"Default encryption plugin\": \"<parallels-default-plugin>\",\r\n    \"Verbose log\": \"off\",\r\n    \"Allow mobile clients\": \"off\",\r\n    \"Proxy connection state\": \"disconnected\",\r\n    \"Direct connection\": \"off\",\r\n    \"Log rotation\": \"on\",\r\n    \"Advanced security mode\": \"off\",\r\n    \"External device auto connect\": \"ask\",\r\n    \"Proxy manager URL\": \"https:\\\/\\\/pax-manager.myparallels.com\\\/xmlrpc\\\/rpc.do\",\r\n    \"Web portal domain\": \"parallels.com\",\r\n    \"Host ID\": \"\",\r\n    \"Allow attach screenshots\": \"on\",\r\n    \"Custom password protection\": \"off\",\r\n    \"License\": {\r\n        \"state\": \"valid\",\r\n        \"key\": \"078-43914-64601-23907-20564-38467\",\r\n        \"restricted\": \"false\"\r\n    },\r\n    \"Hardware Id\": \"{d16e8ca5-3b60-5bbe-aff4-5ba1012dd0cc}\",\r\n    \"Signed In\": \"yes\",\r\n    \"Hardware info\": {\r\n        \"\/dev\/disk0\": {\r\n            \"name\": \"APPLE HDD ST1000DM003 (disk0)\",\r\n            \"type\": \"hdd\"\r\n        },\r\n        \"\/dev\/disk0s1\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"\/dev\/disk0s2\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"\/dev\/disk0s3\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"en0\": {\r\n            \"name\": \"Ethernet\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en1\": {\r\n            \"name\": \"Wi-Fi\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"p2p0\": {\r\n            \"name\": \"p2p0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"awdl0\": {\r\n            \"name\": \"awdl0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en2\": {\r\n            \"name\": \"en2\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en3\": {\r\n            \"name\": \"en3\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"vnic0\": {\r\n            \"name\": \"vnic0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"vnic1\": {\r\n            \"name\": \"vnic1\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"\/dev\/cu.Bluetooth-Incoming-Port\": {\r\n            \"name\": \"\\\/dev\\\/cu.Bluetooth-Incoming-Port\",\r\n            \"type\": \"serial\"\r\n        }\r\n    }\r\n}";
  if (!res.out.empty()) {
    QString output;

    for (auto s : res.out) {
      output += s;
    }

    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    QJsonObject data = doc.object();
    QJsonObject hardware_info = data["Hardware info"].toObject();
    QString vm_storage = data["VM home"].toString();

    if (vm_storage != "") {
      CSettingsManager::Instance().set_parallels_vm_storage(vm_storage);
    }

    qDebug() << hardware_info;

    QStringList keys = hardware_info.keys();
    for(int i = 0; i < keys.count(); ++i){
        QString key = keys.at(i);
        QString name, type;
        QJsonObject obj = hardware_info[key].toObject();

        if (obj["type"].toString() == "net") {
          interfaces.push_back(key);
          qDebug() << "interface: "
                   << key;
        }
    }
  }

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS)
      return interfaces;

  installer_is_busy.unlock();
  return interfaces;
}

QStringList CSystemCallWrapper::hyperv_interfaces() {
  installer_is_busy.lock();

  qDebug("Getting list of bridged interfaces hyperv");

  QStringList interfaces;
  QString cmd = "powershell.exe";
  QString empty;

  system_call_wrapper_error_t cr;

  if ((cr = CSystemCallWrapper::which(cmd, empty)) != SCWE_SUCCESS) {
    installer_is_busy.unlock();
    qCritical() << "Now found powershell";
    return interfaces;
  }

  QStringList tmp_dir = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

  if (!tmp_dir.empty()) {
    QString tmpScript =
        tmp_dir[0] + QDir::separator() + "get_switches.ps1";
    QFileInfo file(tmpScript);

    if (!file.exists())
      QFile::copy(":/hub/get_switches.ps1", tmpScript);

    if (file.exists()) {
      QProcess proc;
      QString output;

      proc.start(QString("%1 -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass \"&('%2')\"").arg(cmd, tmpScript));
      proc_controller started_proc(proc);
      proc.waitForFinished();

      output = QString(proc.readAllStandardOutput());

      qDebug() << "Got hyper-v interface: "
               << output;

      if (proc.exitCode() != 0) {
        qCritical() << "Failed script hyperv bridge: "
                    << "exit code: "
                    << proc.exitCode()
                    << proc.readAllStandardError();

        installer_is_busy.unlock();
        return interfaces;
      }

      QJsonDocument json_doc = QJsonDocument::fromJson(output.toUtf8());
      QJsonArray arr = json_doc.array();

      foreach (const QJsonValue & val, arr) {
        QJsonObject obj = val.toObject();
        interfaces.append(obj["Name"].toString());

        qInfo() << "BRIDGE NAME: "
                << obj["Name"].toString();
      }

      installer_is_busy.unlock();
      return interfaces;
    }
  }

  qCritical() << "Tmp directory not valid"
              << " OR can't copy script file to tmp dir";

  installer_is_busy.unlock();
  return interfaces;
}

QStringList CSystemCallWrapper::libvirt_interfaces() {
  installer_is_busy.lock();

  qDebug("Getting list of bridged interfaces libvirt");

  QStringList interfaces;
  QString cmd = "virsh";
  QString empty;

  system_call_wrapper_error_t cr;

  if ((cr = CSystemCallWrapper::which(cmd, empty)) != SCWE_SUCCESS) {
    installer_is_busy.unlock();
    return interfaces;
  }

  QStringList args;
  args << "iface-list";

  qDebug() << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 60000);
  QString output;

  for (auto s : res.out) {
    output += s;
  }

  qDebug() << "Listing libvirt interfaces result:"
           << "exit code: "
           << res.exit_code
           << "result: "
           << res.res
           << "output:"
           << output;

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS)
      return interfaces;

  // parse bridge interface
  QStringList parse_me = res.out;
  unsigned i = 0;
  for (auto line : parse_me) {
    if (i > 1) {
      QStringList list = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

      QString bridge = list.at(0);
      QString state = list.at(1);

      if (state == "active" && bridge != "lo") {
        interfaces.push_back(bridge);
      }

      qDebug() << bridge
               << state;

    }
    i++;
  }

  installer_is_busy.unlock();
  return interfaces;
}

QStringList CSystemCallWrapper::virtualbox_interfaces() {
  /*#1 how to get bridged interfaces
   * using command VBoxManage get list of all bridged interfaces
   * */
  installer_is_busy.lock();
  qDebug("Getting list of bridged interfaces virtualbox");
  QString vb_version;
  CSystemCallWrapper::oracle_virtualbox_version(vb_version);
  static QStringList interfaces;
  interfaces.clear();
  if (vb_version == "undefined") {
    installer_is_busy.unlock();
    return interfaces;
  }
  QString path = CSettingsManager::Instance().oracle_virtualbox_path();
  QDir dir(path);
  dir.cdUp();
  path = dir.absolutePath();
  path += "/VBoxManage";
  QStringList args;
  args << "list" << "bridgedifs";
  qDebug()<<path<<args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(path, args, true, true, 60000);
  QString output;
  for (auto s : res.out) {
    output += s;
  }
  qDebug() << "Listing interfaces result:"
           << "exit code:" << res.exit_code
           << "result:" << res.res
           << "output:" << output;
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS)
      return interfaces;
  //time to parse data
  QStringList parse_me = res.out;
  QString flag;
  QString value;
  QString last_name;
  bool reading_value = false;
  for (auto s : parse_me){
      flag = value = "";
      reading_value = false;
      for (int i=0; i < s.size(); i++){
          if(s[i]=='\r')continue;
          if(reading_value){
              value += s[i];
              continue;
          }
          if(s[i] == ':'){
              flag = value;
              value = "";
              continue;
          }
          if(s[i] != ' '){
              if(value == "" && !flag.isEmpty())
                  reading_value = true;
              value+=s[i];
          }
      }
      if(flag == "Name")
          last_name = value;
      if(flag == "Status" && value == "Up")
          interfaces.push_back(last_name);
  }
  installer_is_busy.unlock();
  return interfaces;
}
//////////////////////////////////////////////////////////////////////
void CSystemCallWrapper::vagrant_plugins_list(std::vector<std::pair<QString, QString> > &plugins){
    qDebug()
            <<"get list of installed vagrant plugins";
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QString tmp;
    QStringList args;
    args << "plugin"
         << "list";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    qDebug()
            <<"List of vagrant plugins installed"
            <<"exit  code:"<<res.exit_code
            <<"result code:"<<res.res
            <<"output"<<res.out;
    plugins.clear();
    if(res.exit_code != 0 || res.res != 0)
        return;
    QStringList plugin;
    for (auto s : res.out){
        plugin.clear();
        plugin = s.split(' ');
        if(plugin.size() >= 2) {
          tmp = plugin[1];
          tmp.remove(QRegularExpression("/[^0-9.]/g"));
          plugins.push_back(std::make_pair(plugin[0], tmp));
        }
    }
}
//////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::vagrant_latest_box_version(const QString &box,
                                                                           const QString &provider,
                                                                           QString &version){
    qDebug() << "get the latest box version of " << box << "provider: " << provider;
    vagrant_version(version);
    if (version == "undefined"){
        version = QObject::tr("Install Vagrant first");
        return SCWE_CREATE_PROCESS;
    }
    version = "undefined";
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args << "box" << "list";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if (res.res != SCWE_SUCCESS || res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    QStringList box_info;
    for (auto s : res.out){
        s.remove(QRegExp("[(,)]"));
        box_info = s.split(" ", QString::SkipEmptyParts);
        if(box_info.size() < 3) continue;
        if(box_info[0] == box && box_info[1] == provider)
            version = box_info[2];
    }
    while(!version[version.size() - 1].isLetterOrNumber()) version.remove(version.size() -1 , 1);
    return SCWE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::vagrant_add_box(const QString &box,
                                                                const QString &provider,
                                                                const QString &box_dir) {
    qDebug() << "adding vagrant box:" << box << "for provider:" << provider
             << "box dir:" << box_dir;
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args << "box"
         << "add" << box
         << "--provider" << VagrantProvider::Instance()->CurrentVal()
         << box_dir
         << "--force";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
    qDebug () << "finished addding box:" << box
              << "result code: " << res.res
              << "exit code: " << res.exit_code
              << "error message: " << res.out;
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    return SCWE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////
bool CSystemCallWrapper::check_peer_management_components(){
    QString version;
    vagrant_version(version);
    if(version == "undefined"){
        CNotificationObserver::Error(QObject::tr("Cannot create a peer without Vagrant installed in your system. "
                                                 "To install, go to the menu > Components."), DlgNotification::N_ABOUT);
        return false;
    }
    oracle_virtualbox_version(version);
    if(version == "undefined"){
        CNotificationObserver::Error(QObject::tr("You must have at least one hypervisor installed, to control peers."), DlgNotification::N_ABOUT);
        return false;
    }
    std::vector<std::pair<QString, QString> >plugins;
    CSystemCallWrapper::vagrant_plugins_list(plugins);
    static std::vector<QString> required_plugin = {"vagrant-subutai, vagrant-vbguest"};
    for (auto  plugin : required_plugin){
        if(std::find_if(plugins.begin(), plugins.end(),[plugin](const std::pair<QString, QString> &installed_plugin){
            return plugin == installed_plugin.first;}) == plugins.end()){
            CNotificationObserver::Info(QObject::tr("Installing the vagrant plugin: %1").arg(plugin), DlgNotification::N_NO_ACTION);
            vagrant_plugin(plugin, QString("install"));
        }
    }
    return true;
}

system_call_wrapper_error_t CSystemCallWrapper::join_to_p2p_swarm(
    const QString &hash, const QString &key, const QString &ip, int swarm_base_interface_id) {

  if (is_in_swarm(hash)) return SCWE_SUCCESS;

  if (!CCommons::IsApplicationLaunchable(
          CSettingsManager::Instance().p2p_path()))
    return SCWE_P2P_IS_NOT_RUNNABLE;

  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "start"
       << "-ip" << ip
       << "-key" << key
       << "-hash" << hash;
  if (swarm_base_interface_id != -1)
       args << "-dev" << base_interface_name() + QString::number(swarm_base_interface_id);

  qDebug() << "ARGS=" << args;

  system_call_res_t res;
  res = ssystem_th(cmd, args, true, true, 1000 * 60 * 2); // timeout 2 min
  if (res.res != SCWE_SUCCESS){
      qCritical()
          << QString("Join to p2p swarm failed for swarm_hash: %1. Code : %2. Output: ")
             .arg(hash).arg(res.exit_code) << res.out;
  }

  if (res.out.size() == 1 && res.out.at(0).indexOf("[ERROR]") != -1) {
    QString err_msg = res.out.at(0);
    qCritical("%s for swarm_hash : %s", err_msg.toStdString().c_str(), hash.toStdString().c_str());
    res.res = SCWE_CANT_JOIN_SWARM;
  }

  if (res.exit_code != 0) {
    qCritical()
        << QString("Join to p2p swarm failed for swarm_hash: %1. Code : %2. Output: ")
           .arg(hash).arg(res.exit_code) << res.out;
    res.res = SCWE_CREATE_PROCESS;
  }

  return res.res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::leave_p2p_swarm(
    const QString &hash) {
  if (hash == nullptr || !is_in_swarm(hash)) return SCWE_SUCCESS;
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "stop"
       << "-hash" << hash;
  system_call_res_t res =
      ssystem_th(cmd, args, false, true);  // we don't need output. AHAHA // wtf is this????
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t restart_p2p_service_internal(int *res_code,restart_p2p_type type);

template<>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_LINUX> >(
    int *res_code, restart_p2p_type type) {
  *res_code = RSE_MANUAL;
  do {
    system_call_wrapper_error_t scr;
    QString systemctl_path;
    scr = CSystemCallWrapper::which("systemctl", systemctl_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find systemctl command. "
                                    "Make sure that the command exists "
                                    "on your system or reinstall Linux.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      break;
    }
    QStringList args;
    if (type == STARTED_P2P) {
      args << "stop" << "subutai-p2p";
    } else if (type == STOPPED_P2P) {
      args << "start" << "subutai-p2p";
    } else {
      args << "restart" << "subutai-p2p";
    }
    system_call_res_t res = CSystemCallWrapper::ssystem(systemctl_path, args, true, true, 60000);
    if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Couldn't reload subutai-p2p.service. ec = %1, err = %2")
                               .arg(res.exit_code)
                               .arg(CSystemCallWrapper::scwe_error_to_str(res.res));
      qCritical() << err_msg;
      break;
    }
    *res_code = RSE_SUCCESS;
  } while (0);

  return SCWE_SUCCESS;
}
/*************************/

template<>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_WIN> >(
    int *res_code, restart_p2p_type type) {
  QString cmd("sc");
  QStringList args0, args1;
  args0 << "stop"
        << "Subutai P2P";
  args1 << "start"
        << "Subutai P2P";
  system_call_res_t res;
  if(type != STOPPED_P2P)
      res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true);
  if(type != STARTED_P2P)
    res = CSystemCallWrapper::ssystem_th(cmd, args1, true, true);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

template<>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_MAC> >(
    int *res_code, restart_p2p_type type) {
  QString cmd("osascript");
  QStringList args;
  type == UPDATED_P2P ?
      args << "-e"
           << "do shell script \"launchctl unload "
              "/Library/LaunchDaemons/io.subutai.p2p.daemon.plist;"
              " launchctl load /Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
              " with administrator privileges" :
              type == STOPPED_P2P ?
                  args << "-e"
                       << "do shell script \"launchctl load "
                          "/Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
                          " with administrator privileges" :
                  args << "-e"
                       << "do shell script \"launchctl unload "
                          "/Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
                          " with administrator privileges";

  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

system_call_wrapper_error_t CSystemCallWrapper::restart_p2p_service(
    int *res_code, restart_p2p_type type) {
  return restart_p2p_service_internal<Os2Type<CURRENT_OS> >(res_code, type);
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::check_container_state(
    const QString &hash, const QString &ip) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show"
       << "-hash" << hash << "-check" << ip;
  system_call_res_t res = ssystem_th(cmd, args, true, true);
  qDebug()
          <<"container state of hash:"<<hash<<"ip:"<<ip<<"exit code:"<<res.exit_code<<"out:"<<res.out;
  return res.exit_code == 0 ? SCWE_SUCCESS : SCWE_CONTAINER_IS_NOT_READY;
}

////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t run_sshkey_in_terminal_internal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &key);

template<>
system_call_wrapper_error_t run_sshkey_in_terminal_internal<Os2Type<OS_LINUX> >(
        const QString &user,
        const QString &ip,
        const QString &port,
        const QString &key) {
  QString str_command;

  if (port.isEmpty()) {
    str_command = QString("%1 %2@%3").arg(CSettingsManager::Instance().ssh_path())
                                           .arg(user)
                                           .arg(ip);
  } else {
    str_command = QString("%1 %2@%3 -p %4").arg(CSettingsManager::Instance().ssh_path())
                                           .arg(user)
                                           .arg(ip)
                                           .arg(port);
  }

  if (!key.isEmpty()) {
    qInfo() << QString("Using %1 ssh key").arg(key);
    str_command += QString(" -i \"%1\" ").arg(key);
  }

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }
  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args = CSettingsManager::Instance().terminal_arg().split(
                       QRegularExpression("\\s"));
  args << QString("%1;bash").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS
                                            : SCWE_SSH_LAUNCH_FAILED;
}
/*********************/

template<>
system_call_wrapper_error_t run_sshkey_in_terminal_internal<Os2Type<OS_MAC> >(
        const QString &user,
        const QString &ip,
        const QString &port,
        const QString &key) {
  QString str_command = QString("%1 %2@%3 -p %4")
                            .arg(CSettingsManager::Instance().ssh_path())
                            .arg(user)
                            .arg(ip)
                            .arg(port);

  if (!key.isEmpty()) {
    qInfo() << QString("Using %1 ssh key").arg(key);
    str_command += QString(" -i '%1' ").arg(key);
  }

  QString cmd;
  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args;
  qInfo("Launch command : %s",str_command.toStdString().c_str());
  args << "-e" << QString("Tell application \"%1\"").arg(cmd)
       << "-e" << QString("%1 \"%2\"").arg(CSettingsManager::Instance().terminal_arg(), str_command)
       << "-e" << "activate"
       << "-e" << "end tell";
  return QProcess::startDetached(QString("osascript"), args) ? SCWE_SUCCESS
                                            : SCWE_SSH_LAUNCH_FAILED;
}
/*********************/

template<>
system_call_wrapper_error_t run_sshkey_in_terminal_internal<Os2Type<OS_WIN> >(const QString &user,
                                                                           const QString &ip,
                                                                           const QString &port,
                                                                           const QString &key) {
  //these args are used. we just make compiller happy.
  UNUSED_ARG(user);
  UNUSED_ARG(ip);
  UNUSED_ARG(port);
  UNUSED_ARG(key);
#ifdef RT_OS_WINDOWS
  QString str_command = QString("\"%1\" %2@%3 -p %4")
                            .arg(CSettingsManager::Instance().ssh_path())
                            .arg(user)
                            .arg(ip)
                            .arg(port);

  if (!key.isEmpty()) {
    str_command += QString(" -i \"%1\" ").arg(key);
  }


  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }
  cmd = CSettingsManager::Instance().terminal_cmd();

  QString known_hosts = CSettingsManager::Instance().ssh_keys_storage() +
                        QDir::separator() + "known_hosts";
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  QString cmd_args =
      QString("\"%1\" /k \"%2 -o UserKnownHostsFile=\"%3\"\"").arg(cmd).arg(str_command).arg(known_hosts);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(nullptr, cmd_args_lpwstr, nullptr, nullptr, FALSE, 0, nullptr,
                          nullptr, &si, &pi);
  if (!cp) {
    qCritical(
        "Failed to create process %s. Err : %d", cmd.toStdString().c_str(),
        GetLastError());
    return SCWE_SSH_LAUNCH_FAILED;
  }
#endif
  return SCWE_SUCCESS;
}
/*********************/

////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::run_sshkey_in_terminal(
    const QString &user, const QString &ip, const QString &port,
    const QString &key) {
   return run_sshkey_in_terminal_internal<Os2Type<CURRENT_OS> >(user, ip, port, key);
}
//////////////////////////////////////////////////////////////////////////////
template<class  OS>
system_call_wrapper_error_t vagrant_command_terminal_internal(const QString &dir,
                                                              const QString &command,
                                                              const QString &name);

template<>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_MAC> > (const QString &dir,
                                                                                 const QString &command,
                                                                                 const QString &name) {
  QMutexLocker locker(&vagrant_is_busy);
  if(command.isEmpty()) {
    return SCWE_CREATE_PROCESS;
  }

  UNUSED_ARG(name);
  QString tmp;
  tmp = name;
  tmp.remove("\"");
  QString str_command = QString("cd \"%1\"; %2 %3 2> %4_%5;\n").arg(dir,
                                                              CSettingsManager::Instance().vagrant_path(),
                                                              command,
                                                              tmp, *(command.split(" ").begin()));
  if (command == "reload") {
    str_command += QString("%1 provision 2>> %3_%2; \n").arg(CSettingsManager::Instance().vagrant_path(), command, tmp);
  }

  str_command += QString("echo finished > %1_finished; "
                         "echo 'Peer finished to %1 with following errors:'; "
                         "cat %2_%1; "
                         "echo 'Press any key to finish:'; "
                         "read -s -n 1; exit").arg(*(command.split(" ").begin()), tmp);

  QByteArray apple_script;
  QByteArray bash_script = QString(
    "#!/bin/bash\n"
    "%1").arg(str_command).toUtf8();

  QString bash_script_path = CSystemCallWrapper::write_script("vagrant_script.sh", bash_script);

  if (bash_script_path.isEmpty()) {
    return SCWE_CREATE_PROCESS;
  }

  QString cmd = CSettingsManager::Instance().terminal_cmd();

  qInfo("Vagrant launch command : %s", str_command.toStdString().c_str());

  if (cmd == "iTerm") {
    apple_script = QString("Tell application \"iTerm\"\n"
                           "activate\n"
                           "tell current window\n"
                           "create window with default profile\n"
                           "tell current session\n"
                           "write text \"%1\"\n"
                           "end tell\n"
                           "end tell\n"
                           "end tell\n").arg(bash_script_path).toUtf8();
  } else {
    apple_script = QString("tell application \"Terminal\"\n"
                           "activate\n"
                           "do script \"%1\"\n"
                           "end tell\n").arg(bash_script_path).toUtf8();
  }

  QString apple_script_path = CSystemCallWrapper::write_script("apple_script.scpt", apple_script);
  QStringList args;

  if (apple_script_path.isEmpty()) {
    return SCWE_CREATE_PROCESS;
  }

  args << apple_script_path;
  qInfo() << "Vagrant luanch apple script:"
          << args;

  system_call_wrapper_error_t res = QProcess::startDetached(QString("osascript"), args) ? SCWE_SUCCESS
                                                                                        : SCWE_CREATE_PROCESS;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));
  return res;
}

template<>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_LINUX> >(const QString &dir,
                                                                                  const QString &command,
                                                                                  const QString &name) {
  QMutexLocker locker(&vagrant_is_busy);

  if (command.isEmpty()) {
    return SCWE_CREATE_PROCESS;
  }

  UNUSED_ARG(name);
  QString str_command = QString("cd \"%1\"; %2 %3 2> %4_%5;").arg(dir,
                                                                CSettingsManager::Instance().vagrant_path(),
                                                                command,
                                                                name, *(command.split(" ").begin()));
  if (command == "reload") {
    str_command += QString("%1 provision 2>> %3_%2; ").arg(CSettingsManager::Instance().vagrant_path(), command, name);
  }

  str_command += QString("echo finished > %1_finished; echo 'Peer finished to %1 with following errors:'; cat %2_%1; echo 'Press any key to finish:'; read -s -n 1; exit").arg(*(command.split(" ").begin()), name);

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }
  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args = CSettingsManager::Instance().terminal_arg().split(
                       QRegularExpression("\\s"));
  args << QString("%1").arg(str_command);
  system_call_wrapper_error_t res = QProcess::startDetached(cmd, args) ? SCWE_SUCCESS
                                                                       : SCWE_CREATE_PROCESS;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 * 1000));
  return res;
}

template<>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_WIN> >(const QString &dir,
                                                                                  const QString &command,
                                                                                  const QString &name) {
  QMutexLocker locker(&vagrant_is_busy);
UNUSED_ARG(name);
UNUSED_ARG(dir);
UNUSED_ARG(command);
#ifdef RT_OS_WINDOWS

  if (command.isEmpty()) {
    return SCWE_CREATE_PROCESS;
  }

  QStorageInfo root_storage(QCoreApplication::applicationDirPath());
  QStorageInfo peer_drive(dir);
  QString str_command;

  if (root_storage.rootPath() == peer_drive.rootPath()) {
    str_command = QString("cd \"%1\" & %2 %3 2> %4_%5 & ").arg(dir,
                                                                CSettingsManager::Instance().vagrant_path(),
                                                                command,
                                                                name, *(command.split(" ").begin()));
  } else {
    QString drive = peer_drive.rootPath();
    drive.remove("/");
    drive.remove("\\");
    str_command = QString("%1 & cd \"%2\" & %3 %4 2> %5_%6 & ").arg(drive, dir,
                                                                CSettingsManager::Instance().vagrant_path(),
                                                                command,
                                                                name, *(command.split(" ").begin()));
  }

  if (command == "reload") {
    str_command += QString("%1 provision 2>> %3_%2 & ").arg(CSettingsManager::Instance().vagrant_path(), command, name);
  }

  qInfo() << "Vagrant start stop commands: "
          << str_command;

  str_command += QString("echo finished > %1_finished & echo Peer finished to %1 with following messages: "
                         "& type %2_%1 &"
                         "& cd / & set /p DUMMY=Hit ENTER to finish... & exit").arg(*(command.split(" ").begin()), name);

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }
  cmd = CSettingsManager::Instance().terminal_cmd();

  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  QString cmd_args =
      QString("\"%1\" /k %2").arg(cmd).arg(str_command);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(nullptr, cmd_args_lpwstr, nullptr, nullptr, FALSE, 0, nullptr,
                          nullptr, &si, &pi);
  if (!cp) {
    qCritical(
        "Failed to create process %s. Err : %d", cmd.toStdString().c_str(),
        GetLastError());
    return SCWE_CREATE_PROCESS;
  }
#endif
  // sleep for 10 seconds until vm machine state is locked
  QTime dieTime= QTime::currentTime().addSecs(10);
      while (QTime::currentTime() < dieTime)
          QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::vagrant_command_terminal(const QString &dir,
                                                                         const QString &command,
                                                                         const QString &name) {
    return vagrant_command_terminal_internal<Os2Type<CURRENT_OS> >(dir, command, name);
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::vagrant_box_update(const QString &box, const QString &provider) {
    qDebug() << "updating vagrant box: " << box << "provider:" << provider;

    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args << "box"
         << "update"
         << "--box"
         << box
         << "--provider"
         << VagrantProvider::Instance()->CurrentVal();
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
    qDebug() << "updating vagrant box: " << box << "provider:" << provider
             << "finished with" << "exit code: " << res.exit_code
             << "output" << res.out;
    if (res.exit_code != 0){
        res.res = SCWE_CREATE_PROCESS;
    }
    return res.res;
}

system_call_wrapper_install_t CSystemCallWrapper::vagrant_box_remove(const QString &box,
                                                                   const QString &provider) {
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;   // vagrant box remove box_name --provider provider_name --all
  QString version;

  args << "box"
       << "remove"
       << box
       << "--provider"
       << provider
       << "--all";

  system_call_wrapper_install_t res_install;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 60 * 1000 * 5);

  qDebug() << "Removing vagrant box: "
           << box
           << " provider: "
           << provider
           << " finished with exit code: "
           << res.exit_code
           << " output: "
           << res.out;
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  res_install.res = res.res;

  if (res_install.res == SCWE_SUCCESS) {
    vagrant_latest_box_version(box, provider, version);
    res_install.version = version;
  }

  return res_install;
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t uninstall_p2p_internal(const QString &dir, const QString &file_name);

template<class OS>
system_call_wrapper_error_t install_p2p_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_p2p_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name){
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 5);
  qDebug() << "p2p installation has finished"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

template<>
system_call_wrapper_error_t install_p2p_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){
    QString cmd("msiexec");
    QStringList args0;
    args0 << "set_working_directory"
          << dir
          << "/i"
          << file_name
          << "/qn";

    qDebug()
            <<"Installing package:"
            <<args0;

    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true,  1000 * 60 * 3);
    if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
        res.res = SCWE_CREATE_PROCESS;
    else{
        int exit_code;
        CSystemCallWrapper::restart_p2p_service(&exit_code, UPDATED_P2P);
        if(exit_code != 0)
            res.res = SCWE_CREATE_PROCESS;
    }
    return res.res;
}

template<>
system_call_wrapper_error_t install_p2p_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
    QString file_info = dir + QDir::separator() + file_name;
    QString pkexec_path;
    system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
        QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
        qCritical() << err_msg;
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        return SCWE_WHICH_CALL_FAILED;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath =
        lst_temp[0] + QDir::separator() + "p2p_installer.sh";

    qDebug() << tmpFilePath;

    QFile tmpFile(tmpFilePath);
    if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
      QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                        .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QByteArray install_script = QString(
                                    "#!/bin/bash\n"
                                    "dpkg -i %1")
                                    .arg(file_info)
                                    .toUtf8();

    if (tmpFile.write(install_script) != install_script.size()) {
      QString err_msg = QObject::tr("Couldn't write install script to temp file")
                               .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    tmpFile.close();  // save

    if (!QFile::setPermissions(
            tmpFilePath,
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
      QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    system_call_res_t cr2;
    QStringList args2;
    args2 << sh_path
          << tmpFilePath;
    qDebug()
            <<"Installation of p2p started:"
            <<"args: "<<args2;
    cr2 = CSystemCallWrapper::ssystem(pkexec_path, args2, true, true, 97);
    qDebug()
            <<"Installation of p2p finished:"
            <<"error code: "<<cr2.exit_code
            <<"output: "<<cr2.out
            <<"result: "<<cr2.res;
    tmpFile.remove();
    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
      return SCWE_CREATE_PROCESS;
    return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_p2p_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // pkexec apt-get remove -y subutai-p2p
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "apt-get"
       << "remove"
       << "-y"
       << p2p_package_name();

  scr = CSystemCallWrapper::ssystem(QString("pkexec"), args, false, true, 60000);

  qDebug() << "Uninstallation of P2P finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;
  if (scr.exit_code != 0 || scr.exit_code != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall P2P err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_p2p_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  // p2p path: /Applications/SubutaiP2P.app
  if (!QDir("/Applications/SubutaiP2P.app").exists()) {
    qDebug() << "Can't find p2p path: /Applications/SubutaiP2P.app";
    return SCWE_COMMAND_FAILED;
  }

  // sudo launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist
  QString cmd("osascript");
  QStringList args;

  args << "-e"
       << QString("do shell script \"launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist; "
                  "rm -rf /Applications/SubutaiP2P.app/ \" with administrator privileges");
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  if (res.exit_code != 0) res.res = SCWE_CREATE_PROCESS;

  return res.res;
}

template<>
system_call_wrapper_error_t uninstall_p2p_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // wmic product where name="Subutai P2P" call uninstall
  QString cmd("wmic");
  QStringList args;

  args << "product"
       << "where"
       << QString("name=\"%1\"").arg(p2p_package_name())
       << "call"
       << "uninstall";

  qDebug() << "Uninstall P2P: "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);

  qDebug() << "Uninstall P2P finished: "
           << "exit code: "
           << res.exit_code
           << "output: "
           << res.out;

  if (res.exit_code != 0)
    return SCWE_CREATE_PROCESS;

  return res.res;
}

system_call_wrapper_install_t CSystemCallWrapper::install_p2p(const QString &dir, const QString &file_name) {

    installer_is_busy.lock();
    system_call_wrapper_install_t res;
    QString version;

    res.res = install_p2p_internal<Os2Type<CURRENT_OS> >(dir, file_name);

    if (res.res == SCWE_SUCCESS) {
      p2p_version(version);
      res.version = version;
    }

    installer_is_busy.unlock();
    return res;
}

system_call_wrapper_install_t CSystemCallWrapper::update_p2p(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_install_t res_v;
  system_call_wrapper_error_t res;
  QString version;

  res = update_p2p_linux(dir, file_name);
  res_v.res = res;

  if (res_v.res == SCWE_SUCCESS) {
    p2p_version(version);
    res_v.version = version;
  }
  installer_is_busy.unlock();
  return res_v;
}

system_call_wrapper_error_t CSystemCallWrapper::update_p2p_linux(const QString &dir, const QString &file_name) {
  QString file_info = dir + QDir::separator() + file_name;
  QString pkexec_path;
  system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QString sh_path;
  scr = CSystemCallWrapper::which("sh", sh_path);
  if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
  }

  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

  if (lst_temp.empty()) {
    QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  QString tmpFilePath =
      lst_temp[0] + QDir::separator() + "p2p_updater.sh";

  qDebug() << tmpFilePath;

  QFile tmpFile(tmpFilePath);
  if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
    QString err_msg = QObject::tr("Couldn't create updater script temp file. %1")
                      .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  QByteArray install_script = QString(
                                  "#!/bin/bash\n"
                                  "apt -y -f install %1")
                                  .arg(file_info)
                                  .toUtf8();

  if (tmpFile.write(install_script) != install_script.size()) {
    QString err_msg = QObject::tr("Couldn't write updater script to temp file")
                             .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  tmpFile.close();  // save

  if (!QFile::setPermissions(
          tmpFilePath,
          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
              QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
              QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
              QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
    QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  system_call_res_t cr2;
  QStringList args2;
  args2 << sh_path
        << tmpFilePath;
  qDebug()
          <<"updater of p2p started:"
          <<"args: "<<args2;
  cr2 = CSystemCallWrapper::ssystem(pkexec_path, args2, true, true, 97);
  qDebug()
          <<"updater of p2p finished:"
          <<"error code: "<<cr2.exit_code
          <<"output: "<<cr2.out
          <<"result: "<<cr2.res;
  tmpFile.remove();
  if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
    return SCWE_CREATE_PROCESS;

  QString p2p_path;
  scr = CSystemCallWrapper::which("p2p", p2p_path);
  if (scr == SCWE_SUCCESS) {
    CSettingsManager::Instance().set_p2p_path(p2p_path);
  } else {
    qCritical() << "Unable to find p2p command.";
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_p2p(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_p2p_internal<Os2Type <CURRENT_OS> >(dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    p2p_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t install_x2go_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"hdiutil attach %1; "
                  "cp -R /Volumes/x2goclient/x2goclient.app /Applications/x2goclient.app \" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  return res.res;
}

template<>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){
    QString cmd(dir + QDir::separator() + file_name);
    QStringList args0;
    args0 << "/S";

    qDebug()
            <<"Installing package x2go:"
            <<args0;

    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true,  97);
    qDebug()
            <<"Installing package x2go finished"
            <<"cmd:"<<cmd
            <<"exit code"<<res.exit_code
            <<"result:"<<res.res
            <<"output"<<res.out;
    if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
        res.res = SCWE_CREATE_PROCESS;
    return res.res;
}

template<>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  static QString script_name = "x2goinstall.sh";
  QByteArray install_script = QString(
                                  "#!/bin/bash\n"
                                  "apt-get install --yes x2goclient\n"
                                  "if [ $? -gt 0 ]\n"
                                  "then\n"
                                  "dpkg --configure -a\n"
                                  "apt-get install -y -f\n"
                                  "apt-get install --yes x2goclient\n"
                                  "fi\n"
                                  ).toUtf8();

  system_call_res_t res = CSystemCallWrapper::run_script(script_name, install_script);

  qDebug() << "installation of x2goclient finished "
           << "error code: "
           << res.exit_code
           << "output: "
           << res.out;

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::install_x2go(const QString &dir, const QString &file_name) {
    installer_is_busy.lock();
    QString version;
    system_call_wrapper_install_t res;
    res.res = install_x2go_internal<Os2Type <CURRENT_OS> >(dir, file_name);

    if (res.res == SCWE_SUCCESS) {
      x2go_version(version);
      res.version = version;
    }

    installer_is_busy.unlock();
    return res;
}

template<class OS>
system_call_wrapper_error_t uninstall_x2go_internal();

template<>
system_call_wrapper_error_t uninstall_x2go_internal< Os2Type <OS_LINUX> >() {
  // pkexec apt-get remove -y x2goclient --purge
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "apt-get"
       << "remove"
       << "-y"
       << "x2goclient"
       << "--purge";

  scr = CSystemCallWrapper::ssystem(QString("pkexec"), args, false, true, 60000);

  qDebug() << "Uninstallation of x2goclient finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;
  if (scr.exit_code != 0 || scr.res != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall X2GO-Client err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_x2go_internal< Os2Type <OS_WIN> >() {
  QString uninstall_string;
  QString cmd("REG");
  QStringList args;

  args << "QUERY"
       << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\x2goclient"
       << "/v"
       << "UninstallString";

  qDebug() << "Uninstall x2goclient query: "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);

  qDebug() << "got x2goclient query"
           << "exit code: "
           << res.exit_code
           << "result code: "
           << res.res
           << "output: "
           << res.out;

  if (res.res != SCWE_SUCCESS || res.exit_code != 0 || res.out.empty()) {
    qCritical() << "x2goclient query failed";
    return SCWE_CREATE_PROCESS;
  }

  for (QString s: res.out) {
    if (s.contains("UninstallString")) {
      std::string sstd = s.toStdString();
      int ind = (int) sstd.find('"');
      if (ind != -1) {
        uninstall_string = QString(sstd.substr(ind, sstd.find('"', ind + 1)).c_str());
        break;
      }
    }
  }

  if (uninstall_string.size() == 0) {
    qCritical() << "x2goclient uninstall command is empty.";
    return SCWE_CREATE_PROCESS;
  }

  cmd = uninstall_string;
  args.clear();

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);

  qDebug() << "x2goclient uninstalling process finished."
           << "command:" << cmd
           << "exit code:" << res.exit_code
           << "output:" << res.out;

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "x2goclient uninstall failed";
    return SCWE_COMMAND_FAILED;
  }

  return res.res;
}

template<>
system_call_wrapper_error_t uninstall_x2go_internal< Os2Type <OS_MAC> >() {
  // rm -rf /Applications/x2goclient.app
  if (!QDir("/Applications/x2goclient.app").exists()) {
    qDebug() << "Can't find x2goclient path: /Applications/x2goclient.app";
    return SCWE_COMMAND_FAILED;
  }

  QString cmd("osascript");
  QStringList args;

  args << "-e"
       << QString("do shell script \"%1\" "
                  "with administrator privileges")
          .arg("rm -rf /Applications/x2goclient.app");

  qDebug() << "uninstall x2goclient internal osx"
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true);

  if (res.exit_code != 0) {
    return SCWE_COMMAND_FAILED;
  }

  return res.res;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_x2go() {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_x2go_internal<Os2Type <CURRENT_OS> >();

  if (res.res == SCWE_SUCCESS) {
    x2go_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t install_vagrant_internal(const QString &dir, const QString &file_name);

template<class OS>
system_call_wrapper_error_t uninstall_vagrant_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_vagrant_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);
  if (res.exit_code != 0){
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

template<>
system_call_wrapper_error_t install_vagrant_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
    QString cmd("msiexec");
    QStringList args0;
    args0 << "set_working_directory"
          << dir
          << "/i"
          << file_name;

    qDebug()
            <<"Installing package vagrant:"
            <<args0;

    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true, 97);
    qDebug()
            <<"vagrant installation finished. results: "
            <<"exit code: "<<res.exit_code
            <<"result code: "<<res.res
            <<"output: "<<res.out;
    if(res.exit_code != 0 && res.exit_code != 3010 && res.res == SCWE_SUCCESS)
        res.res = SCWE_CREATE_PROCESS;
    return res.res;
}
template<>
system_call_wrapper_error_t install_vagrant_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name){
    QString file_info = dir + QDir::separator() + file_name;
    QString pkexec_path;
    system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
        QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
        qCritical() << err_msg;
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        return SCWE_WHICH_CALL_FAILED;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath =
        lst_temp[0] + QDir::separator() + "vagrant_installer.sh";

    qDebug() << tmpFilePath;

    QFile tmpFile(tmpFilePath);
    if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
      QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                        .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QByteArray install_script = QString(
                                    "#!/bin/bash\n"
                                    "dpkg -i %1\n"
                                    "if [ $? -gt 0 ]\n"
                                    "then\n"
                                    "dpkg --remove --force-remove-reinstreq %1\n"
                                    "apt-get install -y -f\n"
                                    "dpkg -i %1\n"
                                    "else\n"
                                    "rm %1\n"
                                    "fi")
                                    .arg(file_info)
                                    .toUtf8();

    if (tmpFile.write(install_script) != install_script.size()) {
      QString err_msg = QObject::tr("Couldn't write install script to temp file")
                               .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    tmpFile.close();  // save

    if (!QFile::setPermissions(
            tmpFilePath,
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
      QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    system_call_res_t cr2;
    QStringList args2;
    args2 << sh_path << tmpFilePath;

    qDebug()<<"Vagrant installation started"
            <<"pkexec_path:"<<pkexec_path
            <<"args2:"<<args2;

    cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 1000 * 60 * 60);
    qDebug()<<"Vagrant installation finished:"
            <<"exit code:"<<cr2.exit_code
            <<"result code:"<<cr2.res
            <<"output:"<<cr2.out;
    tmpFile.remove();
    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Couldn't install vagrant err = %1")
                               .arg(CSystemCallWrapper::scwe_error_to_str(cr2.res));
      qCritical() << err_msg;
      return SCWE_CREATE_PROCESS;
    }

    return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_vagrant_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // pkexec apt-get remove -y vagrant
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "apt-get"
       << "remove"
       << "-y"
       << "vagrant";

  scr = CSystemCallWrapper::ssystem_th(QString("pkexec"), args, false, true, 1000 * 60 * 60);

  qDebug() << "Uninstallation of Vagrant finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;

  if (scr.exit_code != 0 || scr.res != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall Vagrant err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return scr.res;
}

template<>
system_call_wrapper_error_t uninstall_vagrant_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  //
  // rm -rf /opt/vagrant
  // rm -f /usr/local/bin/vagrant
  // sudo pkgutil --forget com.vagrant.vagrant
  QString cmd("osascript");
  QStringList args;

  args << "-e"
       << QString("do shell script \"%1; %2; %3\" "
                  "with administrator privileges")
          .arg("rm -rf /opt/vagrant")
          .arg("rm -f /usr/local/bin/vagrant")
          .arg("pkgutil --forget com.vagrant.vagrant");

  qDebug() << "uninstall vagrant internal osx"
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true);

  if (res.exit_code != 0) {
    return SCWE_COMMAND_FAILED;
  }

  return res.res;
}

template<>
system_call_wrapper_error_t uninstall_vagrant_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // get vagrant product code
  // wmic product where "Name like '%vagrant%'" get IdentifyingNumber
  QString cmd = "wmic";
  QStringList args;
  args
    << "product" << "where"
    << "Name like 'Vagrant'"
    << "get" << "IdentifyingNumber";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "got product code of vagrant"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) return SCWE_CREATE_PROCESS;
  QString product_code;
  if (res.out.size() == 3) {
    product_code = res.out[1];
    product_code = product_code.trimmed();
  }
  if (product_code.isEmpty()) return SCWE_CREATE_PROCESS;
  // use msiexec to delete virtualbox
  cmd = "msiexec";
  args.clear();
  args << "/x"
       << product_code;
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "remove vagrant finished"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) return SCWE_CREATE_PROCESS;
  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::install_vagrant(const QString &dir, const QString &file_name){
   installer_is_busy.lock();
   QString version;
   system_call_wrapper_install_t res;
   res.res = install_vagrant_internal<Os2Type <CURRENT_OS> >(dir, file_name);

   if (res.res == SCWE_SUCCESS) {
     vagrant_version(version);
     res.version = version;
   }

   installer_is_busy.unlock();
   return res;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_vagrant(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_vagrant_internal<Os2Type <CURRENT_OS> >(dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    vagrant_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}

QString CSystemCallWrapper::write_script(const QString& file_name, const QByteArray &script) {
  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
  QString empty;

  if (lst_temp.empty()) {
    QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return empty;
  }

  QString tmpFilePath =
      lst_temp[0] + QDir::separator() + file_name;

  qDebug() << "write script path: "
           << tmpFilePath;

  QFile tmpFile(tmpFilePath);

  // If script file exist, should remove
  if (tmpFile.exists())
    tmpFile.remove();

  if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
    QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                      .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return empty;
  }

  if (tmpFile.write(script) != script.size()) {
    QString err_msg = QObject::tr("Couldn't write script to temp file")
                             .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return empty;
  }

  tmpFile.close();  // save

  if (!QFile::setPermissions(
          tmpFilePath,
          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
              QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
              QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
              QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
    QString err_msg = QObject::tr("Couldn't set exe permission to install script file");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
    return empty;
  }

  return tmpFilePath;
}

system_call_res_t CSystemCallWrapper::run_script(const QString &file_name, const QByteArray &script) {
  system_call_res_t res;
  QString pkexec_path;
  system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    res.res = SCWE_WHICH_CALL_FAILED;
    return res;
  }

  QString sh_path;
  scr = CSystemCallWrapper::which("sh", sh_path);
  if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      res.res = SCWE_WHICH_CALL_FAILED;
      return res;
  }

  QString tmpFilePath = CSystemCallWrapper::write_script(file_name, script);

  if (tmpFilePath.isEmpty()) {
    res.res = SCWE_CREATE_PROCESS;
    return res;
  }

  qDebug() << "script path: "
           << tmpFilePath;

  QFile tmpFile(tmpFilePath);

  system_call_res_t cr2;
  QStringList args2;
  args2 << sh_path << tmpFilePath;
  cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 60 * 1000 * 10); // 10 min
  qDebug() << "Run script " << file_name
           << cr2.exit_code
           << cr2.res
           << cr2.out;

  tmpFile.remove(); // remove script from tmp directory

  return cr2;
}

system_call_wrapper_install_t CSystemCallWrapper::install_vagrant_libvirt() {
  // install dependency packages
  // more info here https://github.com/vagrant-libvirt/vagrant-libvirt#vagrant-libvirt-provider
  system_call_wrapper_install_t res;
  static QString file_name = "vagrant_libvirt_installer.sh";

  QByteArray script = QString(
    "#!/bin/bash\n"
    "apt-get -y build-dep ruby-libvirt\n"
    "apt-get -y install qemu libvirt-bin ebtables dnsmasq\n"
    "apt-get -y install libxslt-dev libxml2-dev libvirt-dev zlib1g-dev ruby-dev\n").toUtf8();

  system_call_res_t cr2;
  cr2 = CSystemCallWrapper::run_script(file_name, script);
  res.res = cr2.res;

  qDebug() << "Vagrant libvirt dependency package script executed: "
           << "exit code: " << cr2.exit_code
           << "output: " << cr2.out
           << "result: " << cr2.res;

  static QString plugin = "vagrant-libvirt";
  static QString command = "install";
  res = vagrant_plugin(plugin, command);

  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CSystemCallWrapper::vagrant_plugin
/// \param name of vagrant plugin
/// \param command (we use for "vagrant plugin command")
/// accepted commands are: uninstall, install, update
/// \return
///
system_call_wrapper_install_t CSystemCallWrapper::vagrant_plugin(const QString &name,
                                                               const QString &command) {
  installer_is_busy.lock();
  qDebug() << QString("Vagrant plugin %1 %2 started.")
              .arg(command)
              .arg(name);

  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args << "plugin"
       << command // might be: uninstall, install, update
       << name;

  system_call_res_t res_t = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 1000 * 60 * 10); // 10 minutes timeout

  qDebug() << QString("Vagrant plugin %1 %2 is finished.")
              .arg(command)
              .arg(name)
           << " Exist code: "
           << res_t.exit_code
           << " Result: "
           << res_t.res
           << " Output: "
           << res_t.out;

  if(res_t.res == SCWE_SUCCESS && res_t.exit_code != 0)
          res_t.res = SCWE_CREATE_PROCESS;

  QString version;
  system_call_wrapper_install_t res;
  res.res = res_t.res;

  if (res.res == SCWE_SUCCESS) {
    vagrant_plugin_version(version, name);
    res.version = version;
  }

  installer_is_busy.unlock();
  return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                         INSTALLATION VAGRANT VMWARE UTILITY                                        ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_vmware_utility_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_vmware_utility_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;

  args << "-e"
       << QString("do shell script \"hdiutil attach %1;\"").arg(file_path);

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);

  qDebug() << "Mount Vagrant VMware Utility dmg"
           << " exit code: "
           << rs.exit_code
           << " res code: "
           << rs.res
           << "output: "
           << rs.out;

  if (rs.exit_code != 0 && rs.res != SCWE_SUCCESS) {
    qDebug() << QString("Failed mount %1 dmg file").arg(file_name);
    return SCWE_CREATE_PROCESS;
  }

  args.clear();

  args << "-e"
       << QString("do shell script \"installer -pkg /Volumes/Vagrant\\\\ VMware\\\\ Utility/*.pkg -target /\" with administrator privileges");

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 10);

  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }

  return res.res;
}

template <>
system_call_wrapper_error_t install_vmware_utility_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  QString cmd("msiexec");
  QStringList args0;
  args0 << "set_working_directory"
        << dir
        << "/i"
        << file_name
        << "/qn";

  qDebug()
          <<"Starting installation Vagrant VMware Utility:"
          <<args0;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true,  1000 * 60 * 10);

  if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
      res.res = SCWE_CREATE_PROCESS;

  return res.res;
}

template <>
system_call_wrapper_error_t install_vmware_utility_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  QString pkexec_path;
  system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall "
                                  "the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QString sh_path;
  scr = CSystemCallWrapper::which("sh", sh_path);

  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists "
                                  "on your system or reinstall Linux.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

  if (lst_temp.empty()) {
    QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your "
                                  "file system is setup correctly and fix any issues.");
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  QString tmpFilePath =
      lst_temp[0] + QDir::separator() + "vmware_utility_installer.sh";

  qDebug() << tmpFilePath;

  QFile tmpFile(tmpFilePath);
  if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
    QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                      .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  QString file_path = dir + QDir::separator() + file_name;

  // check file path
  QFileInfo check_file(file_path);
  if (!check_file.exists()) {
    qDebug() << "Vagrant VMware Utility file path doesn't exist: "
             << file_path;
    return SCWE_CREATE_PROCESS;
  }

  QByteArray install_script = QString(
                                  "#!/bin/bash\n"
                                  "dpkg -i %1\n"
                                  "if [ $? -gt 0 ]\n"
                                  "then\n"
                                  "dpkg --remove --force-remove-reinstreq %1\n"
                                  "apt-get install -y -f\n"
                                  "dpkg -i %1\n"
                                  "else\n"
                                  "rm %1\n"
                                  "fi\n")
                                  .arg(file_path)
                                  .toUtf8();

  qDebug() << "Vagrant VMware utility installation "
           << "dir: "
           << dir
           << " file_name: "
           << file_name;

  if (tmpFile.write(install_script) != install_script.size()) {
    QString err_msg = QObject::tr("Couldn't write install script to temp file")
                             .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  tmpFile.close();  // save

  if (!QFile::setPermissions(
          tmpFilePath,
          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
              QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
              QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
              QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {

    QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  system_call_res_t cr2;
  QStringList args2;

  args2 << sh_path << tmpFilePath;

  cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 1000 * 60 * 10); // 10 minutes

  qDebug() << "Vagrant VMware utility installation finished"
           << "error code:"
           << cr2.exit_code
           << "output: "
           << cr2.out
           << "result: "
           << cr2.res;

  if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
    return SCWE_CREATE_PROCESS;

  tmpFile.remove();

  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::install_vmware_utility(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString vmware_ver;
  QString version;
  system_call_wrapper_install_t res;
  vmware_version(vmware_ver);

  if (vmware_ver == "undefined") {
    qCritical() << "Install VMware first";
    res.res = SCWE_CREATE_PROCESS;
    return res;
  }


  res.res = install_vmware_utility_internal<Os2Type<CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    vmware_utility_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                         UNINSTALLATION VAGRANT VMWARE UTILITY                                        ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t uninstall_vmware_utility_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t uninstall_vmware_utility_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  QString cmd("osascript");
  QStringList args;
  static QString file_path = "/opt/vagrant-vmware-desktop";

  // Uninstall script
  args << "-e"
       << QString("do shell script \"/opt/vagrant-vmware-desktop/bin/vagrant-vmware-utility service uninstall; /bin/rm -Rf %1\" "
                  "with administrator privileges")
          .arg(file_path);

  qDebug() << "Uninstallation Vagrant VMware Utility"
           << args;

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args, true, true);

  if (rs.exit_code != 0) {
    return SCWE_COMMAND_FAILED;
  }

  return rs.res;
}

template <>
system_call_wrapper_error_t uninstall_vmware_utility_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // wmic product where name="Vagrant VMware Utility" call uninstall
  QString cmd("wmic");
  QStringList args;

  args << "product"
       << "where"
       << QString("name=\"%1\"").arg("Vagrant VMware Utility")
       << "call"
       << "uninstall";

  qDebug() << "Uninstalling Vagrant VMware Utility: "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);

  qDebug() << "Uninstall Vagrant VMware Utility finished: "
           << "exit code: "
           << res.exit_code
           << "output: "
           << res.out;

  if (res.exit_code != 0)
    return SCWE_CREATE_PROCESS;

  return res.res;
}

template <>
system_call_wrapper_error_t uninstall_vmware_utility_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // pkexec apt-get remove -y vagrant-vmware-utility
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "apt-get"
       << "purge"
       << "-y"
       << "vagrant-vmware-utility";

  scr = CSystemCallWrapper::ssystem_th(pkexec_path, args, true, true, 97);

  qDebug() << "Uninstallation of Vagrant VMware Utility finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;

  if (scr.exit_code != 0 || scr.res != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall Vagrant VMware Utility err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return scr.res;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_vmware_utility(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString vmware_ver;
  QString version;
  vmware_version(vmware_ver);
  system_call_wrapper_install_t res;

  if (vmware_ver == "undefined") {
    qCritical() << "Install VMware first";
    res.res = SCWE_CREATE_PROCESS;
    return res;
  }

  res.res = uninstall_vmware_utility_internal<Os2Type<CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    vmware_utility_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                         INSTALLATION VMWARE FUSION, WORKSTATION                                    ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_vmware_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_vmware_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  // Installation process
  // 1. Atttach VMware Fusion dmg file
  // 2. Start installation process
  // open -a /Volumes/VMware\ Fusion/VMware\ Fusion.app/
  qDebug() << "Installation VMware Fusion";

  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;

  args << "-e"
       << QString("do shell script \"hdiutil attach %1; "
                  "cp -R /Volumes/VMware\\\\ Fusion/VMware\\\\ Fusion.app/ /Applications/VMware\\\\ Fusion.app; open -a VMware\\\\ Fusion.app;\" with administrator privileges").arg(file_path);

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 60); // 1 hour timeout

  if (res.res != SCWE_SUCCESS && res.exit_code != 0) {
    qCritical() << "Failed installation VMware Fusion: "
                << res.res
                << " exit code: "
                << res.exit_code
                << res.out;

    return SCWE_CREATE_PROCESS;
  }

  return res.res;
}

template <>
system_call_wrapper_error_t install_vmware_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  // VMware-workstation-full-14.1.2-8497320.exe /s /v"/qn EULAS_AGREED=1
  QString cmd(dir + QDir::separator() + file_name);
  QStringList args;

  qDebug() << "Installing package VMware: "
           << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);

  qDebug() << "Installing package VMware finished: "
           << " cmd: "
           << cmd
           << "args: "
           << args
           << " exit code: "
           << res.exit_code
           << " result: "
           << res.res
           << " output: "
           << res.out;

  if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
      res.res = SCWE_CREATE_PROCESS;

  return res.res;
}

template <>
system_call_wrapper_error_t install_vmware_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  QString pkexec_path;
  system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QString sh_path;
  scr = CSystemCallWrapper::which("sh", sh_path);

  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QString tmpFilePath =
      dir + QDir::separator() + file_name;

  qDebug() << tmpFilePath;

  qDebug() << "VMware installation "
           << "dir: "
           << dir
           << " file_name: "
           << file_name;

  system_call_res_t cr2;
  QStringList args2;

  args2 << sh_path << tmpFilePath << "--console" << "--required" << "--ignore-errors" << "--eulas-agreed";

  cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 1000 * 60 * 60 * 1); // 1 hour timeout

  qDebug() << "VMware worstation installation finished"
           << "error code:"
           << cr2.exit_code
           << "output: "
           << cr2.out
           << "result: "
           << cr2.res;

  if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
    return SCWE_CREATE_PROCESS;

  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::install_vmware(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = install_vmware_internal<Os2Type<CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    vmware_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                 UNINSTALLATION VMWARE                                              ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class OS>
system_call_wrapper_error_t uninstall_vmware_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t uninstall_vmware_internal<Os2Type <OS_MAC> > (const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // close VMware Fusion
  // osascript -e "Tell application \"VMware Fusion\" to quit"

  // clean following folders
  // /Library/Preferences/VMware\ Fusion/
  // /Library/Application Support/VMware
  // /Applications/VMware\ Fusion.app/

  if (!QDir("/Applications/VMware Fusion.app/").exists()) {
    qDebug() << "Can't find VMware Fusion path: /Applications/VMware Fusion.app";
    return SCWE_COMMAND_FAILED;
  }

  QString cmd("osascript");
  QStringList args;

  // Quit Application VMware Fusion
  args << "-e"
       << "Tell application \"VMware Fusion\" to quit";
  system_call_res_t res = CSystemCallWrapper::ssystem(cmd, args, true, true);

  if (res.exit_code != 0) {
    qDebug() << "Failed to quit VMware Fusion application";
    return SCWE_COMMAND_FAILED;
  }

  args.clear();

  // Clean VMware Fusion folders
  args << "-e"
       << QString("do shell script \"%1\" "
                  "with administrator privileges")
          .arg("rm -rf /Applications/VMware\\\\ Fusion.app/; "
               "rm -rf /Library/Preferences/VMware\\\\ Fusion/; "
               "rm -rf /Library/Preferences/VMware*/;"
               "rm -rf /Library/Application\\\\ Support/VMware*;");

  qDebug() << "Uninstallation VMware Fusion internal"
           << args;

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args, true, true);

  if (rs.exit_code != 0) {
    return SCWE_COMMAND_FAILED;
  }

  return rs.res;
}

template <>
system_call_wrapper_error_t uninstall_vmware_internal<Os2Type <OS_WIN> > (const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // wmic product where "Name like '%VMware%'" call uninstall
  QString cmd("wmic");
  QStringList args;

  args << "product"
       << "where"
       << QString("Name like '%VMware%'")
       << "call"
       << "uninstall";

  qDebug() << "Uninstall VMware: "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);

  qDebug() << "Uninstall VMware finished: "
           << "exit code: "
           << res.exit_code
           << "output: "
           << res.out;

  if (res.exit_code != 0)
    return SCWE_CREATE_PROCESS;

  return res.res;
}

template <>
system_call_wrapper_error_t uninstall_vmware_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // pkexec vmware-installer -u vmware-workstation --console --required --eulas-agreed
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "vmware-installer"
       << "-u"
       << "vmware-workstation"
       << "--gtk"
       << "--required"
       << "--eulas-agreed"
       << "-I";

  scr = CSystemCallWrapper::ssystem_th(QString("pkexec"), args, true, true, 97);

  qDebug() << "Uninstallation of VMware finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;

  if (scr.exit_code != 0 || scr.res != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall VMware Workstation err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return scr.res;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_vmware(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_vmware_internal<Os2Type<CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    vmware_version(version);
    res.version = version;
  }
  installer_is_busy.unlock();

  return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                 INSTALLATION Parallels Desktop                                     ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_install_t CSystemCallWrapper::install_parallels(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  qDebug() << "Installation Parallels Desktop"
           << dir
           << file_name;

  QString cmd("osascript");
  QStringList args;
  QString version;
  QString file_path  = dir + QDir::separator() + file_name;

  args << "-e"
       << QString("do shell script \"hdiutil attach %1; "
                  "cp -R /Volumes/Parallels\\\\ Desktop\\\\ 14/Parallels\\\\ Desktop.app/ /Applications/Parallels\\\\ Desktop.app/; "
                  "open -a Parallels\\\\ Desktop.app;\" with administrator privileges").arg(file_path);

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  system_call_wrapper_install_t res_install;
  res_install.res = res.res;

  if (res.res != SCWE_SUCCESS && res.exit_code != 0) {
    qCritical() << "Failed installation Parallels Desktop: "
                << res.res
                << " exit code: "
                << res.exit_code
                << res.out;

    res_install.res = SCWE_CREATE_PROCESS;
    return res_install;
  }

  if (res_install.res == SCWE_SUCCESS) {
    parallels_version(version);
    res_install.version = version;
  }

  installer_is_busy.unlock();
  return res_install;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                 UNINSTALLATION Parallels Desktop                                     ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////

system_call_wrapper_install_t CSystemCallWrapper::uninstall_parallels(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  QString version;
  system_call_wrapper_install_t res_install;
  res_install.version = "undefined";

  if (!QDir("/Applications/Parallels Desktop.app/").exists()) {
    qDebug() << "Can't find Parallels Desktop path: /Applications/Parallels Desktop.app";
    res_install.res = SCWE_CREATE_PROCESS;
    return res_install;
  }

  QString cmd("osascript");
  QStringList args;

  // Quit Application VMware Fusion
  args << "-e"
       << "tell application \"Parallels Desktop.app\" to quit";
  system_call_res_t res = CSystemCallWrapper::ssystem(cmd, args, true, true, 10000);
  res_install.res = res.res;

  if (res.exit_code != 0) {
    qDebug() << "Failed to quit Parallels Desktop application";
    res_install.res = SCWE_COMMAND_FAILED;
    return res_install;
  }

  args.clear();

  // Clean Parallels Desktop folders
  args << "-e"
       << QString("do shell script \"%1\" "
                  "with administrator privileges")
          .arg("rm -rf /Applications/Parallels\\\\ Desktop.app/; ");

  qDebug() << "Uninstallation Parallels Desktop"
           << args;

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args, true, true);
  qDebug() << "parallels uninstall finished: "
           << rs.exit_code
           << rs.out
           << rs.res;

  if (rs.exit_code != 0) {
    res_install.res = SCWE_COMMAND_FAILED;
    return res_install;
  }

  if (res_install.res == SCWE_SUCCESS) {
    std::this_thread::sleep_for(std::chrono::milliseconds(3 * 1000));
    parallels_version(version);
    res_install.version = version;
  }

  installer_is_busy.unlock();
  return res_install;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t install_oracle_virtualbox_internal(const QString &dir, const QString &file_name);
template<>
system_call_wrapper_error_t install_oracle_virtualbox_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name){
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  qDebug()
          <<"Starting installation of oracle virtualbox"
          <<cmd
          <<args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  qDebug()
          <<"Installation of oracle virtualbox is finished"
          <<"error :"<<res.exit_code
          <<"output :"<<res.out;
  if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
      res.res = SCWE_CREATE_PROCESS;
  return res.res;
}
template<>
system_call_wrapper_error_t install_oracle_virtualbox_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){
    QString cmd(dir + QDir::separator() + file_name);
    QStringList args0;
    args0 << "--silent";

    qDebug()
            <<"Installing package oralce:"
            <<args0
            <<cmd;

    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true,  97);
    qDebug()
            <<"Installing package oracle finished"
            <<"cmd:"<<cmd
            <<"exit code"<<res.exit_code
            <<"result:"<<res.res
            <<"output"<<res.out;
    if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
        res.res = SCWE_CREATE_PROCESS;
    return res.res;
}
template<>
system_call_wrapper_error_t install_oracle_virtualbox_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name){
    QString pkexec_path;
    system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
        QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
        qCritical() << err_msg;
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        return SCWE_WHICH_CALL_FAILED;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath =
        lst_temp[0] + QDir::separator() + "vb_installer.sh";

    qDebug() << tmpFilePath;

    QFile tmpFile(tmpFilePath);
    if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
      QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                        .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }
    QByteArray install_script = QString(
      "#!/bin/bash\n"
      "apt-get install -y dkms\n"
      "if [ $? -gt 0 ]\n"
      "then\n"
      "apt-get install -y -f\n"
      "apt-get install -y dkms\n"
      "fi\n"
      "apt-get install -y libcurl3\n"
      "if [ $? -gt 0 ]\n"
      "then\n"
      "apt-get install -y -f\n"
      "apt-get install -y libcurl3\n"
      "fi\n"
      "apt-get install -y linux-header-`uname -r`\n"
      "if [ $? -gt 0 ]\n"
      "then\n"
      "apt-get install -y -f\n"
      "apt-get install -y linux-header-`uname -r`\n"
      "fi\n"
      "cd %1\n"
      "dpkg -i %2\n"
      "if [ $? -gt 0 ]\n"
      "then\n"
      "apt-get install -y -f\n"
      "dpkg -i %2\n"
      "fi\n"
      "if [ $? -gt 0 ]\n"
      "then\n"
      "dpkg --remove --force-remove-reinstreq %2\n"
      "fi\n").arg(dir, file_name).toUtf8();

    if (tmpFile.write(install_script) != install_script.size()) {
      QString err_msg = QObject::tr("Couldn't write install script to temp file")
                               .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    tmpFile.close();  // save

    if (!QFile::setPermissions(
            tmpFilePath,
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
      QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    system_call_res_t cr2;
    QStringList args2;
    args2 << sh_path << tmpFilePath;
    cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 97);
    qDebug()
            <<"virtualbox oracle installation finished"
            <<"error code:"<<cr2.exit_code
            <<"output: "<<cr2.out
            <<"result: "<<cr2.res;

    tmpFile.remove();

    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
      return SCWE_CREATE_PROCESS;

    return SCWE_SUCCESS;
}
system_call_wrapper_install_t CSystemCallWrapper::install_oracle_virtualbox(const QString &dir, const QString &file_name){
    installer_is_busy.lock();
    QString version;
    system_call_wrapper_install_t res;
    res.res = install_oracle_virtualbox_internal<Os2Type<CURRENT_OS> > (dir, file_name);

    if (res.res == SCWE_SUCCESS) {
      oracle_virtualbox_version(version);
      res.version = version;
    }

    installer_is_busy.unlock();
    return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class  OS>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal(const QString &dir, const QString &file_name);
template<>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal<Os2Type<OS_MAC> > (const QString &dir, const QString &file_name){
  qDebug() << "uninstall virtualbox on mac";
  // run downloaded uninstallation script easy
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"chmod +x %1; %1 --unattended\" with administrator privileges").arg(file_path);
  qDebug() << "ARGS=" << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  qDebug()
          <<"Uninstallation of oracle virtualbox is finished"
          <<"error :"<<res.exit_code
          <<"output :"<<res.out;
  if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
      res.res = SCWE_CREATE_PROCESS;
  return res.res;
}
template<>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal<Os2Type<OS_LINUX> >(const QString &dir, const QString &file_name){
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  qDebug() << "uninstalling virtualbox on linux";
  // first close(kill) all running instances of VB
  QString cmd("ps");
  QStringList args;
  args << "-A";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  qDebug() << "got process IDs of all running vb"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  QStringList pid_vb;
  QString pid;
  if (res.res == SCWE_SUCCESS && res.exit_code == 0) {
    for (QString s : res.out) {
      if (!s.contains("VB")) continue;
      pid = "";
      while (s.size() > 0 && s[0].isSpace()) s.remove(0, 1);
      int i = 0;
      while (i < s.size() && !s[i].isSpace()) {
        pid += s[i];
        i++;
      }
      pid_vb.push_back(pid);
    }
  }
  // after getting pid of all running vb process
  // close each of them and delete vb
  QString pkexec_path;
  system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
  if (scr != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  QString sh_path;
  scr = CSystemCallWrapper::which("sh", sh_path);
  if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
  }

  QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

  if (lst_temp.empty()) {
    QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  QString tmpFilePath =
      lst_temp[0] + QDir::separator() + "vb_installer.sh";

  qDebug() << tmpFilePath;

  QFile tmpFile(tmpFilePath);
  if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
    QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                      .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }
  QString uninstall_script_str;
  uninstall_script_str += "#!/bin/bash\n";
  for (QString s : pid_vb) {
    uninstall_script_str += "kill -9 " + s + "\n";
  }
  uninstall_script_str += "apt-get remove --purge -y virtualbox-*\n";
  QByteArray uninstall_script = uninstall_script_str.toUtf8();

  if (tmpFile.write(uninstall_script) != uninstall_script.size()) {
    QString err_msg = QObject::tr("Couldn't write uninstall script to temp file")
                             .arg(tmpFile.errorString());
    qCritical() << err_msg;
    CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  tmpFile.close();  // save

  if (!QFile::setPermissions(
          tmpFilePath,
          QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
              QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
              QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
              QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
    QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
    return SCWE_CREATE_PROCESS;
  }

  system_call_res_t cr2;
  QStringList args2;
  args2 << sh_path << tmpFilePath;
  cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 97);
  qDebug()
          <<"virtualbox oracle uninstallation finished"
          <<"error code:"<<cr2.exit_code
          <<"output: "<<cr2.out
          <<"result: "<<cr2.res;

  tmpFile.remove();

  if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
    return SCWE_CREATE_PROCESS;
  return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal<Os2Type<OS_WIN> > (const QString &dir,
                                                                                    const QString &file_name){
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // get virtualbox product code
  // wmic product where "Name like '%Virtualbox%'" get IdentifyingNumber
  QString cmd = "wmic";
  QStringList args;
  args
    << "product" << "where"
    << "Name like '%virtualbox%'"
    << "get" << "IdentifyingNumber";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "got product code of virtualbox"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) return SCWE_CREATE_PROCESS;
  QString product_code;
  if (res.out.size() == 3) {
    product_code = res.out[1];
    product_code = product_code.trimmed();
  }
  if (product_code.isEmpty()) return SCWE_CREATE_PROCESS;
  // use msiexec to delete virtualbox
  cmd = "msiexec";
  args.clear();
  args << "/x"
       << product_code
       << "/qn";
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "remove virtualbox finished"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) return SCWE_CREATE_PROCESS;
  return SCWE_SUCCESS;
}
system_call_wrapper_install_t CSystemCallWrapper::uninstall_oracle_virtualbox(const QString &dir, const QString &file_name){
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_oracle_virtualbox_internal<Os2Type<CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    oracle_virtualbox_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();
  return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_install_t CSystemCallWrapper::uninstall_kvm() {
  QString distribution;
  QString file_script = "kvmuninstall.sh";
  QByteArray script;
  system_call_wrapper_install_t res;
  system_call_res_t cr;

  // Get linux distribution (Debian, Ubuntu, LinuxMint)
  QStringList args;
  args << "-i"
       << "-s";

  cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);

  if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
    distribution = cr.out[0];
    distribution = distribution.trimmed().simplified();
  } else {
    qDebug() << "Failed get linux distribution"
             << cr.out;
    res.res = cr.res;
    return res;
  }

  if (distribution.toLower() == "debian") {
    qDebug() << "KVM uninstalling on debian";
    script = QString("#!/usr/bin/env bash \n"
                     "codename=`lsb_release -c -s` \n"
                     "case \"$codename\" in \n"
                     "\"stretch\")\n"
                       "apt remove -y qemu-kvm libvirt-clients libvirt-daemon-system\n"
                       ";;"
                     "\"jessie\")"
                       "apt-get -y remove qemu-kvm libvirt-bin\n"
                       ";;"
                     "*)"
                       "echo \"Bad or unsupported codename: $codename\";"
                       "exit 1"
                       ";;"
                     "esac"
                     ).toUtf8();
  } else if (distribution.toLower() == "ubuntu") {  // Ubuntu
    qDebug() << "KVM uninstalling on ubuntu";
    QString keramic = "9.10";
    QString lucid = "10.04";
    QString version; // ubuntu version

    // Get Ubuntu version
    args.clear();
    args << "-r"
         << "-s";
    cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);
    if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
      version = cr.out[0];
      version = version.trimmed().simplified();
    } else {
      res.res = cr.res;
      qDebug() << "KVM Failed to get ubuntu version"
               << cr.res
               << cr.exit_code
               << cr.out;
      return res;
    }

    // Install KVM
    if (versionCompare(version.toStdString(), lucid.toStdString()) > 0) {
      // Unstalling KVM for later version of Ubuntu 10.04
      qDebug() << "Unstalling KVM for later version of Ubuntu 10.04";
      script = QString("#!/usr/bin/env bash\n"
                       "apt-get remove -y qemu-kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       ).toUtf8();
    } else if (versionCompare(keramic.toStdString(), version.toStdString()) > 0) {
      // Unstalling KVM for ealier version of Ubuntu 9.10
      qDebug() << "Unstalling KVM for ealier version of Ubuntu 9.10";
      script = QString("#!/usr/bin/env bash\n"
                       "aptitude remove -y kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       ).toUtf8();
    }
  } else if (distribution.toLower() == "linuxmint") {  // LinuxMint
    qDebug() << "KVM uninstalling on linuxmint";
    script = QString("#!/usr/bin/env bash\n"
                     "apt-get remove -y qemu-kvm libvirt-bin bridge-utils\n"
                     ).toUtf8();
  }

  system_call_res_t cr_script = CSystemCallWrapper::run_script(file_script, script);
  res.res = cr_script.res;

  if (cr_script.exit_code == 0 && cr_script.res == SCWE_SUCCESS) {
    QString version;
    CSystemCallWrapper::kvm_version(version);
    res.version = version;

    return res;
  }

  qDebug() << "KVM uninstall kvm script failed"
           << cr_script.res
           << cr_script.exit_code
           << cr_script.out;

  return res;
}

system_call_wrapper_install_t CSystemCallWrapper::install_kvm() {
  QString username = qgetenv("USER");
  QString distribution;
  QString file_script = "kvminstall.sh";
  QByteArray script;
  system_call_wrapper_install_t res;
  system_call_res_t cr;

  if (username.isEmpty())
    username = qgetenv("USERNAME");

  // Get linux distribution (Debian, Ubuntu, LinuxMint)
  QStringList args;
  args << "-i"
       << "-s";

  cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);

  if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
    distribution = cr.out[0];
    distribution = distribution.trimmed().simplified();
    qDebug() << "KVM installation on "
             << distribution
             << " username: "
             << username;
  } else {
    qDebug() << "KVM failed get linux distribution"
             << cr.out;
    res.res = cr.res;
    return res;
  }

  if (distribution.toLower() == "debian") {
    qDebug() << "KVM install on debian";
    script = QString("#!/usr/bin/env bash \n"
                     "codename=`lsb_release -c -s` \n"
                     "case \"$codename\" in \n"
                     "\"stretch\")\n"
                       "apt install -y -f qemu-kvm libvirt-clients libvirt-daemon-system\n"
                       "if [ $? -gt 0 ]\n"
                       "then\n"
                       "dpkg --configure -a\n"
                       "apt-get install -y -f\n"
                       "apt install -y -f qemu-kvm libvirt-clients libvirt-daemon-system\n" // if installation fails, install dependency package and try again
                       "fi\n"
                       "adduser %1 libvirt\n"
                       "adduser %1 libvirt-qemu\n"
                       ";;"
                     "\"jessie\")"
                       "apt-get -y install qemu-kvm libvirt-bin\n"
                       "if [ $? -gt 0 ]\n"
                       "then\n"
                       "dpkg --configure -a\n"
                       "apt-get install -y -f\n"
                       "apt-get -y install qemu-kvm libvirt-bin\n"
                       "fi\n"
                       "adduser %1 kvm\n"
                       "adduser %1 libvirt\n"
                       ";;"
                     "*)"
                       "echo \"Bad or unsupported codename: $codename\";"
                       "exit 1"
                       ";;"
                     "esac"
                     ).arg(username).toUtf8();
  } else if (distribution.toLower() == "ubuntu") {  // Ubuntu
    QString keramic = "9.10";
    QString lucid = "10.04";
    QString version; // ubuntu version

    // Get Ubuntu version
    args.clear();
    args << "-r"
         << "-s";
    cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);
    if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
      version = cr.out[0];
      version = version.trimmed().simplified();
    } else {
      res.res = cr.res;
      qDebug() << "KVM Failed to get ubuntu version"
               << cr.res
               << cr.exit_code
               << cr.out;
      return res;
    }

    // Install KVM
    if (versionCompare(version.toStdString(), lucid.toStdString()) > 0) {
      qDebug() << "KVM installing KVM for later version of Ubuntu 10.04";
      // Installing KVM for later version of Ubuntu 10.04
      script = QString("#!/usr/bin/env bash\n"
                       "apt-get install -y -f qemu-kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       "if [ $? -gt 0 ]\n"
                       "then\n"
                       "dpkg --configure -a\n"
                       "apt-get install -y -f\n"
                       "apt-get install -y -f qemu-kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       "fi\n"
                       "adduser %1 libvirtd\n").arg(username).toUtf8();
    } else if (versionCompare(keramic.toStdString(), version.toStdString()) > 0) {
      qDebug() << "KVM installing KVM for ealier version of Ubuntu 9.10";
      // Installing KVM for ealier version of Ubuntu 9.10
      script = QString("#!/usr/bin/env bash\n"
                       "aptitude install -y -f kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       "if [ $? -gt 0 ]\n"
                       "then\n"
                       "dpkg --configure -a\n"
                       "aptitude install -y -f\n"
                       "aptitude install -y -f kvm libvirt-bin ubuntu-vm-builder bridge-utils\n"
                       "fi\n"
                       "adduser %1 kvm\n"
                       "adduser %1 libvirtd\n").arg(username).toUtf8();
    }
  } else if (distribution.toLower() == "linuxmint") {  // LinuxMint
    qDebug() << "KVM installing linuxmint";
    script = QString("#!/usr/bin/env bash\n"
                     "apt-get install -y -f qemu-kvm libvirt-bin bridge-utils\n"
                     "if [ $? -gt 0 ]\n"
                     "then\n"
                     "dpkg --configure -a\n"
                     "apt-get install -y -f\n"
                     "apt-get install -y -f qemu-kvm libvirt-bin bridge-utils\n"
                     "fi\n"
                     "adduser %1 libvirt\n").arg(username).toUtf8();
  }

  system_call_res_t cr_script = CSystemCallWrapper::run_script(file_script, script);
  res.res = cr_script.res;

  if (cr_script.exit_code == 0 && cr_script.res == SCWE_SUCCESS) {
    QString version;
    CSystemCallWrapper::kvm_version(version);
    res.version = version;

    return res;
  }

  qDebug() << "KVM install kvm script failed"
           << cr_script.res
           << cr_script.exit_code
           << cr_script.out;

  return res;
}

int CSystemCallWrapper::versionCompare(std::string v1, std::string v2) {
  int vnum1 = 0, vnum2 = 0;

  for (size_t i = 0, j = 0; (i < v1.size() || j < v2.size());) {
    while (i < v1.length() && v1[i] != '.') {
        vnum1 = vnum1 * 10 + (v1[i] - '0');
        i++;
    }

    while (j < v2.length() && v2[j] != '.') {
      vnum2 = vnum2 * 10 + (v2[j] - '0');
      j++;
    }

    if (vnum1 > vnum2)
      return 1;
    if (vnum2 > vnum1)
      return -1;

    vnum1 = vnum2 = 0;
    i++;
    j++;
  }

  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t install_chrome_internal(const QString &dir, const QString &file_name);

template<class OS>
system_call_wrapper_error_t uninstall_chrome_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_chrome_internal<Os2Type <OS_MAC> > (const QString &dir, const QString &file_name) {
    qInfo() << "CC started to install google chrome";
    QString cmd("osascript");
    QStringList args;
    QString file_path  = dir + QDir::separator() + file_name;
    args << "-e"
         << QString("do shell script \"hdiutil attach -nobrowse %1; "
                    "cp -R /Volumes/Google\\\\ Chrome/Google\\\\ Chrome.app /Applications/Google\\\\ Chrome.app \" with administrator privileges").arg(file_path);
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
    qDebug() << "Installation of chrome finished"
             << "Result code: " << res.res
             << "Exit code: " << res.exit_code
             << "Output messages" << res.out;
    if(res.exit_code != 0){
        res.res = SCWE_CREATE_PROCESS;
    }
    return res.res;
}
template<>
system_call_wrapper_error_t install_chrome_internal<Os2Type <OS_LINUX> > (const QString& dir, const QString &file_name) {
    QString file_info = dir + QDir::separator() + file_name;
    QString pkexec_path;
    system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
        QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
        qCritical() << err_msg;
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        return SCWE_WHICH_CALL_FAILED;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath =
        lst_temp[0] + QDir::separator() + "chrome_installer.sh";

    qDebug() << tmpFilePath;

    QFile tmpFile(tmpFilePath);
    if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
      QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                        .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QByteArray install_script = QString(
                                    "#!/bin/bash\n"
                                    "dpkg -i %1\n"
                                    "if [ $? -gt 0 ]\n"
                                    "then\n"
                                    "dpkg --remove --force-remove-reinstreq %1\n"
                                    "apt-get install -y -f\n"
                                    "dpkg -i %1\n"
                                    "else\n"
                                    "rm %1\n"
                                    "fi\n")
                                    .arg(file_info)
                                    .toUtf8();

    if (tmpFile.write(install_script) != install_script.size()) {
      QString err_msg = QObject::tr("Couldn't write install script to temp file")
                               .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    tmpFile.close();  // save

    if (!QFile::setPermissions(
            tmpFilePath,
                QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
      QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    system_call_res_t cr2;
    QStringList args2;
    args2 << sh_path << tmpFilePath;

    qDebug()<<"Chrome installation started"
            <<"pkexec_path:"<<pkexec_path
            <<"args2:"<<args2;

    cr2 = CSystemCallWrapper::ssystem(pkexec_path, args2, true, true, 60000);
    qDebug()<<"Chrome installation finished:"
            <<"exit code:"<<cr2.exit_code
            <<"result code:"<<cr2.res
            <<"output:"<<cr2.out;
    tmpFile.remove();
    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Couldn't install chrome err = %1")
                               .arg(CSystemCallWrapper::scwe_error_to_str(cr2.res));
      qCritical() << err_msg;
      return SCWE_CREATE_PROCESS;
    }

    return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t install_chrome_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
    QString cmd(dir + QDir::separator() + file_name);
    QStringList args0;
    args0 << "/install";

    qDebug()
            <<"Installing package chrome:"
            <<args0;

    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args0, true, true,  97);
    qDebug()
            <<"Installing package chrome finished"
            <<"cmd:"<<cmd
            <<"exit code"<<res.exit_code
            <<"result:"<<res.res
            <<"output"<<res.out;
    if(res.exit_code != 0 && res.res == SCWE_SUCCESS)
        res.res = SCWE_CREATE_PROCESS;
    return res.res;
}

template<>
system_call_wrapper_error_t uninstall_chrome_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  QString uninstall_string;
  QString cmd("REG");
  QStringList args;

  args << "QUERY"
       << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Google Chrome"
       << "/v"
       << "UninstallString";

  qDebug() << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);

  qDebug() << "got chrome uninstallstring query: "
           << " exit code: "
           << res.exit_code
           << " result code: "
           << res.res
           << " output: "
           << res.out;

  if (res.res != SCWE_SUCCESS || res.exit_code != 0 || res.out.empty()) {
    qCritical() << "chrome query failed";
    return SCWE_CREATE_PROCESS;
  }

  for (QString s: res.out) {
    if (s.contains("UninstallString")) {
      std::string sstd = s.toStdString();
      int ind = (int) sstd.find('"');
      if (ind != -1) {
        uninstall_string = QString(sstd.substr(ind).c_str());
        break;
      }
    }
  }

  qDebug() << "got uninstall string chrome: "
           << uninstall_string;

  if (uninstall_string.size() == 0) {
    qCritical() << "chrome uninstall command is empty.";
    return SCWE_CREATE_PROCESS;
  }

  std::string sstd = uninstall_string.toStdString();
  uninstall_string = sstd.substr(0, sstd.find('"', 1) + 1).c_str();
  QString args_str = sstd.substr(sstd.find('"', 1) + 1).c_str();

  cmd = uninstall_string;
  args = args_str.split(" ");

  for (QString &s: args) {
    if (s.contains("\r")) {
      sstd = s.toStdString();
      s = sstd.replace(sstd.find("\r"), 2, "").c_str();
    }
  }

  qDebug() << "parsed chrome uninstall command:"
           << "cmd:" << cmd
           << "param:" << args;

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);

  qDebug() << "chrome uninstalling process finished."
           << "exit code:" << res.exit_code
           << "output:" << res.out;

  if (res.exit_code != 0 && res.exit_code != 21) {
    qCritical() << "chrome uninstall failed";
    return SCWE_COMMAND_FAILED;
  }

  return res.res;
}

template<>
system_call_wrapper_error_t uninstall_chrome_internal<Os2Type <OS_LINUX> > (const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  // pkexec apt-get remove -y subutai-p2p
  QString pkexec_path;
  system_call_wrapper_error_t scre = CSystemCallWrapper::which("pkexec", pkexec_path);

  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;

    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);

    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "apt-get"
       << "remove"
       << "-y"
       << "google-chrome-stable";

  scr = CSystemCallWrapper::ssystem(QString("pkexec"), args, false, true, 60000);

  qDebug() << "Uninstallation of Google Chrome finished: "
           << "exit code: "
           << scr.exit_code
           << " output: "
           << scr.out;
  if (scr.exit_code != 0 || scr.exit_code != SCWE_SUCCESS ) {
    QString err_msg = QObject::tr("Couldn't uninstall Google Chrome err = %1")
                             .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_chrome_internal<Os2Type <OS_MAC> > (const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  qDebug() << "uninstall chrome internal";

  // chrome path: /Applications/Google\\\\ Chrome.app/Ï
  if (!QDir("/Applications/Google\\ Chrome.app").exists()) {
    qDebug() << "Can't find chrome path: Applications/Google\\\\ Chrome.app";
    return SCWE_COMMAND_FAILED;
  }

  QString cmd("osascript");
  QStringList args, args_close;

  // close chrome application
  // osascript -e "Tell Application \"Google Chrome\" to quit"
  args_close << "-e"
             << QString("Tell Application \"Google Chrome\" to quit");

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args_close, true, true);

  if (rs.exit_code != 0 && rs.res != SCWE_SUCCESS) {
    qDebug() << "Failed to close chrome app";
  }

  args << "-e"
       << QString("do shell script \"%1\" "
                  "with administrator privileges")
          .arg("rm -rf /Applications/Google\\\\ Chrome.app");

  qDebug() << "uninstall google chrome internal osx"
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true);

  qDebug() << "Ununstall chomr osx finished: "
           << "exit code: "
           << res.exit_code
           << "output: "
           << res.out;
  if (res.exit_code != 0){
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

system_call_wrapper_install_t CSystemCallWrapper::install_chrome(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();

  system_call_wrapper_install_t res;
  QString version;
  res.res = install_chrome_internal <Os2Type <CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    chrome_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_chrome(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_chrome_internal <Os2Type <CURRENT_OS> > (dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    chrome_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();

  return res;
}

template<class OS>
system_call_wrapper_error_t install_e2e_chrome_internal();
template<>
system_call_wrapper_error_t install_e2e_chrome_internal<Os2Type<OS_LINUX> >(){
    QJsonObject json;
    json["external_update_url"] = "https://clients2.google.com/service/update2/crx";
    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath = QString("%1/%2.json")
            .arg(lst_temp[0],
            subutai_e2e_id(CSettingsManager::Instance().default_browser()));

    qDebug() << tmpFilePath;
    QJsonDocument preference_json(json);
    QFile preference_file(tmpFilePath);
    preference_file.open(QFile::WriteOnly);
    preference_file.write(preference_json.toJson());
    preference_file.close();
    QString cmd("pkill");
    QStringList args;
    args << "--oldest"
         << "chrome";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS){
        qCritical() << "Failed to close chrome"
                    << res.exit_code;
        return SCWE_CREATE_PROCESS;
    }
    args.clear();
    cmd = "pkexec";
    args << "bash"
         << "-c"
         << QString("mkdir -p /opt/google/chrome/extensions; "
            "cp -p %1 /opt/google/chrome/extensions/").arg(tmpFilePath);
    res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        qCritical() << "Failed to install e2e"
                    << res.exit_code
                    << res.out;
        return SCWE_CREATE_PROCESS;
    }
    return res.res;
}
template<>
system_call_wrapper_error_t install_e2e_chrome_internal<Os2Type<OS_WIN> >(){
    //cretate key in registry
    QString cmd("REG");
    QStringList args;
    QString ex_id = subutai_e2e_id(CSettingsManager::Instance().default_browser());
    args << "ADD"<< QString("HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Google\\Chrome\\Extensions\\%1").arg(ex_id)
         << "/v" << "update_url"
         << "/t" << "REG_SZ"
         << "/d" << "https://clients2.google.com/service/update2/crx"
         << "/f";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        qCritical() << "failed to install e2e on chrome"
                    << "exit code" << res.exit_code;
        return SCWE_CREATE_PROCESS;
    }
    //close chrome
    cmd = "taskkill";
    args.clear();
    args << "/F" << "/IM" << "chrome.exe" << "/T";
    res = CSystemCallWrapper::ssystem(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS || (res.exit_code != 0 && res.exit_code != 128)){
        qCritical() << "failed to close chrome"
                    << "exit code" << res.exit_code;
        return SCWE_CREATE_PROCESS;
    }
    return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t install_e2e_chrome_internal<Os2Type<OS_MAC> >(){
    QJsonObject json;
    json["external_update_url"] = "https://clients2.google.com/service/update2/crx";
    QStringList home_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

    if (home_path.empty()) {
      QString err_msg = QObject::tr("Couldn't get standard home location");
      qCritical() << err_msg;
      return SCWE_CREATE_PROCESS;
    }

    QString jsonFilePath = QString("%1/Library/Application Support/Google/Chrome/External Extensions/%2.json").arg(home_path[0], subutai_e2e_id(CSettingsManager::Instance().default_browser()));
    QString cmd("osascript");
    QStringList args;
    args << "-e"
         << "tell application \"Google Chrome\" to quit";
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        qCritical() << "Failed to close Chrome"
                    << "Exit code: " << res.exit_code
                    << "Output: " << res.out;
        return SCWE_CREATE_PROCESS;
    }
    args.clear();
    args << "-e"
         << "do shell script \"mkdir -p ~/Library/Application\\\\ Support/Google/Chrome/External\\\\ Extensions\"";
    res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    QJsonDocument preference_json(json);
    QFile preference_file(jsonFilePath);
    preference_file.open(QFile::WriteOnly);
    preference_file.write(preference_json.toJson());
    preference_file.close();
    return res.res;
}
system_call_wrapper_error_t CSystemCallWrapper::install_e2e_chrome(){
    installer_is_busy.lock();
    system_call_wrapper_error_t res = install_e2e_chrome_internal<Os2Type<CURRENT_OS> > ();
    installer_is_busy.unlock();
    return res;
}
system_call_wrapper_install_t CSystemCallWrapper::install_e2e(const QString &dir, const QString &file_name) {
  system_call_wrapper_install_t res;
  QString version;
  QString current_browser = CSettingsManager::Instance().default_browser();
  if (current_browser == "Chrome") {
    res.res = install_e2e_chrome();
  } else if (current_browser == "Firefox") {
    res.res = install_e2e_firefox(dir, file_name);
  } else if (current_browser == "Safari") {
    res.res = install_e2e_safari(dir, file_name);
  }

  if (res.res == SCWE_SUCCESS && current_browser == "Chrome") {
    chrome_last_session();
    QTime dieTime = QTime::currentTime().addSecs(2);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    subutai_e2e_version(version);
    res.version = version;
  } else if (res.res == SCWE_SUCCESS) {
    subutai_e2e_version(version);
    res.version = version;
  }

  return res;
}

template<class OS>
system_call_wrapper_error_t uninstall_e2e_chrome_internal();

template<>
system_call_wrapper_error_t uninstall_e2e_chrome_internal<Os2Type<OS_LINUX>>() {
  qDebug() << "checking if e2e was installed from chrome web store";
  QFile *ext_json = new QFile(QString("/opt/google/chrome/extensions/%1.json").arg(
          subutai_e2e_id(CSettingsManager::Instance().default_browser())));
  if (!ext_json->exists()) {
    qDebug() << "e2e is installed from web store";
    CNotificationObserver::Info(QObject::tr("Due to the Chrome extension policy Control "
                                "Center can not uninstall Subutai E2E extension "
                                "that has been installed via Chrome Web Store."),
                                DlgNotification::N_NO_ACTION);
    return SCWE_CREATE_PROCESS;
  }
  qDebug() << "closing chrome browser";
  QString cmd("pkill");
  QStringList args;
  args << "--oldest"
       << "chrome";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if(res.res != SCWE_SUCCESS){
    qCritical() << "Failed to close chrome"
                << "exit code: " << res.exit_code
                << "output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }
  qDebug() << "removing e2e json file";
  args.clear();
  cmd = "pkexec";
  args << "bash"
       << "-c"
       << QString("rm /opt/google/chrome/extensions/%1.json").arg(
            subutai_e2e_id(CSettingsManager::Instance().default_browser()));
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to uninstall e2e"
                << "exit code: " << res.exit_code
                << "output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }
  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_e2e_chrome_internal<Os2Type<OS_MAC>>() {
  QStringList home_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_path.empty()) {
    QString err_msg = QObject::tr("Couldn't get standard home location");
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  qDebug() << "checking if e2e was installed from chrome web store";

  QString jsonFilePath = QString("%1/Library/Application Support/Google/Chrome/"
                                 "External Extensions/%2.json").arg(home_path[0],
      subutai_e2e_id(CSettingsManager::Instance().default_browser()));

  if (!QFile(jsonFilePath).exists()) {
    qDebug() << "e2e is installed from web store";
    CNotificationObserver::Info(QObject::tr("Due to the Chrome extension policy Control "
                                "Center can not uninstall Subutai E2E extension "
                                "that has been installed via Chrome Web Store."),
                                DlgNotification::N_NO_ACTION);
    return SCWE_CREATE_PROCESS;
  }

  qDebug() << "quitting chrome";

  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << "tell application \"Google Chrome\" to quit";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to close Chrome"
                << "Exit code: " << res.exit_code
                << "Output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }

  qDebug() << "removing e2e from chrome";

  args.clear();
  args << "-e"
       << QString("do shell script \"rm '%1'\"").arg(jsonFilePath);
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to uninstall e2e from chrome"
                << "exit code: " << res.exit_code
                << "output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }
  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_e2e_chrome_internal<Os2Type<OS_WIN>>() {
  QString ex_id = subutai_e2e_id(CSettingsManager::Instance().default_browser());
  //ask if registry key exists
  qDebug() << "checking if e2e was installed from chrome web store";
  QString cmd("REG");
  QStringList args;
  args << "QUERY" << QString("HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Google\\Chrome\\Extensions\\%1").arg(ex_id);
  //REG QUERY "HKLM\Software\Wow6432Node\Google\Chrome\Extensions\ffddnlbamkjlbngpekmdpnoccckapcnh"
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.exit_code == 1) {
    qDebug() << "e2e is installed from web store";
    CNotificationObserver::Info(QObject::tr("Due to the Chrome extension policy Control "
                                "Center can not uninstall Subutai E2E extension "
                                "that has been installed via Chrome Web Store."),
                                DlgNotification::N_NO_ACTION);
    return SCWE_CREATE_PROCESS;
  } else if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "failed to query registry key of e2e on chrome"
                << "exit code: " << res.exit_code
                << "output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }
  //delete registry key
  qDebug() << "removing e2e chrome registry key";
  cmd = "REG";
  args.clear();
  args << "DELETE" << QString("HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Google\\Chrome\\Extensions\\%1").arg(ex_id) << "/f";
  //REG DELETE "HKLM\Software\Wow6432Node\Google\Chrome\Extensions\ffddnlbamkjlbngpekmdpnoccckapcnh" /f
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
      qCritical() << "failed to uninstall e2e on chrome"
                  << "exit code" << res.exit_code
                  << "output: " << res.out;
      return SCWE_CREATE_PROCESS;
  }
  //kill chrome
  qDebug() << "quiting chrome";
  cmd = "taskkill";
  args.clear();
  args << "/F" << "/IM" << "chrome.exe" << "/T";
  res = CSystemCallWrapper::ssystem(cmd, args, true, true, 10000);
  if(res.res != SCWE_SUCCESS || (res.exit_code != 0 && res.exit_code != 128)){
      qCritical() << "failed to close chrome"
                  << "exit code" << res.exit_code
                  << "output: " << res.out;
      return SCWE_CREATE_PROCESS;
  }
  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::uninstall_e2e_chrome() {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_e2e_chrome_internal<Os2Type<CURRENT_OS>>();
  installer_is_busy.unlock();
  return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t install_firefox_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_firefox_internal<Os2Type<OS_LINUX>>(const QString &dir, const QString &file_name) {
  QString distribution;
  static QString script = "firefox_installer.sh";

  QByteArray install_script;
  system_call_res_t cr;

  // Get linux distribution (Debian, Ubuntu, LinuxMint)
  QStringList args;
  args << "-i"
       << "-s";

  cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);

  if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
    distribution = cr.out[0];
    distribution = distribution.trimmed().simplified();
  } else {
    qDebug() << "Firefox installation, failed get linux distribution"
             << cr.out;
    return SCWE_CREATE_PROCESS;
  }

  if (distribution.toLower() == "debian") {
    qDebug() << "Fifox install on debian";
    install_script = QString("#!/usr/bin/env bash \n"
                       "apt install -y -f firefox-esr\n"
                       "if [ $? -gt 0 ]\n"
                       "then\n"
                       "dpkg --configure -a\n"
                       "apt-get install -y -f\n"
                       "apt-get update --fix-missing"
                       "apt-get install -y -f firefox-esr\n" // if installation fails, install dependency package and try again
                       "fi\n"
                     ).toUtf8();
  } else {
    QString file_info =  dir + QDir::separator() + file_name;
    install_script = QString(
                       "#!/bin/bash\n"
                       "dpkg -i %1;"
                       "if test $? -gt 0\n"
                       "then\n"
                       "dpkg --remove --force-remove-reinstreq %2\n"
                       "apt-get install -y -f;\n"
                       "dpkg -i %1;"
                       "else\n"
                       "rm %1\n"
                       "fi")
                      .arg(file_info)
                      .toUtf8();
  }

  system_call_res_t cr2 = CSystemCallWrapper::run_script(script, install_script);;

  if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Couldn't install firefox err = %1")
        .arg(CSystemCallWrapper::scwe_error_to_str(cr2.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t install_firefox_internal<Os2Type<OS_MAC>>(const QString &dir, const QString &file_name) {
  qInfo() << "CC started to install firefox";
  QString cmd("osascript");
  QStringList args;
  QString file_path = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"hdiutil attach -nobrowse %1; "
                  "cp -R /Volumes/Firefox/Firefox.app "
                  "/Applications/Firefox.app\" with administrator privileges")
          .arg(file_path);
  qDebug() << "installing firefox"
           << "cmd:" << cmd
           << "args:" << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "Installation of firefox finished"
           << "Result code:" << res.res
           << "Exit code:" << res.exit_code
           << "Output message:" << res.out;
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

template<>
system_call_wrapper_error_t install_firefox_internal<Os2Type<OS_WIN>>(const QString &dir, const QString &file_name) {
  QString cmd(dir + QDir::separator() + file_name);
  QStringList args0;
  args0 << "/install";
  qDebug() << "installing firefox:" << args0;

  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args0, true, true, 97);
  qDebug() << "Installing firefox finished"
           << "cmd:" << cmd
           << "exit code:" << res.exit_code
           << "result:" << res.res
           << "outpit:" << res.out;
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

system_call_wrapper_install_t CSystemCallWrapper::install_firefox(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = install_firefox_internal<Os2Type<CURRENT_OS>>(dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    firefox_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();
  return res;
}

template<class OS>
system_call_wrapper_error_t uninstall_firefox_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t uninstall_firefox_internal<Os2Type<OS_LINUX>>(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  QString pkexec_path;
  system_call_wrapper_error_t scre =
      CSystemCallWrapper::which("pkexec", pkexec_path);
  if (scre != SCWE_SUCCESS) {
    QString err_msg = QObject::tr(
          "Unable to find pkexec command. You may reinstall "
          "the Control Center or reinstall the PolicyKit.");
    qCritical() << err_msg;
    CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
    return SCWE_WHICH_CALL_FAILED;
  }

  system_call_res_t scr;
  QStringList args;
  args << "-i"
       << "-s";

  system_call_res_t cr = CSystemCallWrapper::ssystem_th("lsb_release", args, true, true, 5000);
  QString distribution;

  if (cr.exit_code == 0 && cr.res == SCWE_SUCCESS && !cr.out.empty()) {
    distribution = cr.out[0];
    distribution = distribution.trimmed().simplified();
  } else {
    qDebug() << "Firefox uninstallation, failed get linux distribution"
             << cr.out;
    return SCWE_CREATE_PROCESS;
  }

  args.clear();
  if (distribution.toLower() == "debian") {
    args << "apt-get"
         << "remove"
         << "-y"
         << "firefox-esr";
  } else {
    args << "apt-get"
         << "remove"
         << "-y"
         << "firefox";
  }

  scr = CSystemCallWrapper::ssystem_th(pkexec_path, args, false, true, 60000);
  qDebug() << "Uninstallation of Firefox finished: "
           << "exit code:" << scr.exit_code
           << "output:" << scr.out;
  if (scr.exit_code != 0 || scr.exit_code != SCWE_SUCCESS) {
    QString err_msg = QObject::tr("Couldn't uninstall firefox err = %1")
        .arg(CSystemCallWrapper::scwe_error_to_str(scr.res));
    qCritical() << err_msg;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_firefox_internal<Os2Type<OS_MAC>>(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  qDebug() << "firefox uninstallation started";

  if (!QDir("/Applications/Firefox.app").exists()) {
    qDebug() << "Can't find firefox path: /Applications/Firefox.app";
    return SCWE_COMMAND_FAILED;
  }

  QString cmd("osascript");
  QStringList args, args_close;
  args_close << "-e"
             << QString("Tell Application \"Firefox\" to quit");
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args_close, true, true, 97);
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qDebug() << "Failed to close firefox";
  }

  args << "-e"
       << QString("do shell script \"%1\" with administrator privileges")
          .arg("rm -rf /Applications/Firefox.app");
  qDebug() << "firefox uninstallation started"
           << "cmd:" << cmd
           << "args:" << args;

  system_call_res_t rs = CSystemCallWrapper::ssystem_th(cmd, args, true, true);
  qDebug() << "Uninstall firefox finished"
           << "exit code:" << rs.exit_code
           << "output:" << rs.out;

  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }

  return res.res;
}

template<>
system_call_wrapper_error_t uninstall_firefox_internal<Os2Type<OS_WIN>>(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  QString path;
  QString cmd("REG");
  QStringList args;
  args << "QUERY"
       << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe";
  qDebug() << "REG QUERY started"
           << "args:" << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "REG QUERY finished:"
           << "exit code:" << res.exit_code
           << "output:" << res.out;
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "REG QUERY failed.";
    return SCWE_CREATE_PROCESS;
  }

  for (QString str: res.out) {
    if (str.contains("Path") && str.contains("REG_SZ")) {
      std::string std_str =
          str.replace(QRegularExpression("[\t\n]+"), "").toStdString();
      int id = std_str.find(" ", std_str.find("REG_SZ"));
      path = str.right(str.size() - id).trimmed();
      break;
    }
  }
  qDebug() << "Got firefox path:" << path;

  QString path_to_uninstaller =
      path + QDir::separator() + "uninstall" + QDir::separator() + "helper.exe";
  QFile uninstaller(path_to_uninstaller);
  if (!uninstaller.exists()) {
    qCritical() << "Control Center can not find Mozilla Firefox "
                   "uninstaller. Please uninstall it manually.";
    return SCWE_CREATE_PROCESS;
  }

  args.clear();
  args << "-ms";
  qDebug() << "Starting firefox uninstallation"
           << "cmd:" << path_to_uninstaller
           << "args:" << args;
  res = CSystemCallWrapper::ssystem_th(path_to_uninstaller, args, true, true, 97);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Firefox uninstallation has failed"
                << "scwe err:" << CSystemCallWrapper::scwe_error_to_str(res.res)
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_firefox(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  QString version;
  system_call_wrapper_install_t res;
  res.res = uninstall_firefox_internal<Os2Type<CURRENT_OS>>(dir, file_name);

  if (res.res == SCWE_SUCCESS) {
    firefox_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();
  return res;
}

// e2e on firefox id: jid1-KejrJUY3AaPCkZ
// e2e firefox extension name: jid1-KejrJUY3AaPCkZ@jetpack.xpi

template<class OS>
system_call_wrapper_error_t install_e2e_firefox_internal(const QString &dir, const QString &file_name);

template<>
system_call_wrapper_error_t install_e2e_firefox_internal<Os2Type<OS_LINUX>>(const QString &dir, const QString &file_name) {
  QString cmd("pkill");
  QStringList args;
  args << "--oldest"
       << "firefox";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS){
    qCritical() << "Failed to close firefox"
                << "exit code:" << res.exit_code;
    return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path += QString("/.mozilla/firefox/%1/extensions/").arg(profile_folder);
  QString cur_dir = dir + QDir::separator() + file_name;
  args.clear();
  cmd = "pkexec";
  args << "bash"
       << "-c"
       << QString("mkdir -p %2; cp -p %1 %2").arg(cur_dir, ext_path);

  qDebug() << "Installing e2e firefox"
           << "cmd:" << cmd
           << "args:" << args;

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "Failed to install e2e"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t install_e2e_firefox_internal<Os2Type<OS_MAC>>(const QString &dir, const QString &file_name) {
  qDebug() << "entered scwe:install_e2e_firefox_internal_os_mac";
  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << "tell application \"Firefox\" to quit";
  qDebug() << "Quitting Firefox";

  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);

  if (res.res != SCWE_SUCCESS || res.exit_code != 0){
    qCritical() << "Failed to close Firefox"
                << "Exit code: " << res.exit_code
                << "Output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  qDebug() << "Got home location.";

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path +=
      QString("/Library/Application\\\\ Support/Firefox/Profiles/%1/extensions/")
      .arg(profile_folder);
  QString cur_dir = dir + QDir::separator() + file_name;
  args.clear();
  cmd = "osascript";
  args << "-e"
       << QString("do shell script \"mkdir -p %2;"
                  "cp %1 %2\"").arg(cur_dir, ext_path);

  qDebug() << "installing e2e firefox"
           << "cmd:" << cmd
           << "args:" << args;

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "Failed to install e2e"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t install_e2e_firefox_internal<Os2Type<OS_WIN>>(const QString &dir, const QString &file_name) {
  qDebug() << "quitting firefox";
  QString cmd("taskkill");
  QStringList args;
  args << "/F" << "/IM" << "firefox.exe" << "/T";
  system_call_res_t res =
      CSystemCallWrapper::ssystem(cmd, args, true, true, 10000);
  if(res.res != SCWE_SUCCESS || (res.exit_code != 0 && res.exit_code != 128)){
      qCritical() << "failed to close firefox"
                  << "exit code" << res.exit_code
                  << "output: " << res.out;
      return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path +=
      QString("\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%1\\extensions\\%2")
      .arg(profile_folder, file_name);
  QString cur_dir = dir + "\\" + file_name;

  QString ext_folder = home_paths_list[0] +
      QString("\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%1\\extensions\\")
      .arg(profile_folder);

  QDir dir_helper("");

  if (!dir_helper.mkpath(ext_folder)) {
    qCritical() << "Failed to create path:" << ext_folder;
    return SCWE_CREATE_PROCESS;
  }

  if (QFile::exists(ext_path)) {
    QFile::remove(ext_path);
  }

  qDebug() << "copying xpi-file from" << cur_dir << "to" << ext_path;

  if (!QFile::copy(cur_dir, ext_path)) {
    qCritical() << "Failed to install e2e";
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::install_e2e_firefox(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = install_e2e_firefox_internal<Os2Type<CURRENT_OS>>(dir, file_name);
  installer_is_busy.unlock();
  return res;
}

template<class OS>
system_call_wrapper_error_t uninstall_e2e_firefox_internal();

template<>
system_call_wrapper_error_t uninstall_e2e_firefox_internal<Os2Type<OS_LINUX>>() {
  QString cmd("pkill");
  QStringList args;
  args << "--oldest"
       << "firefox";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS){
    qCritical() << "Failed to close firefox"
                << "exit code:" << res.exit_code;
    return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path += QString("/.mozilla/firefox/%1/extensions/%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  args.clear();
  cmd = "pkexec";
  args << "bash"
       << "-c"
       << QString("rm -rf %1").arg(ext_path);

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "Failed to ininstall e2e"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_e2e_firefox_internal<Os2Type<OS_MAC>>() {
  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << "tell application \"Firefox\" to quit";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0){
    qCritical() << "Failed to close Firefox"
                << "Exit code: " << res.exit_code
                << "Output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path +=
      QString("/Library/Application\\\\ Support/Firefox/Profiles/%1/extensions/%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  args.clear();
  cmd = "osascript";
  args << "-e"
       << QString("do shell script \"rm -rf %1\"").arg(ext_path);

  qDebug() << "uninstall e2e ggwp" << cmd << args;

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "Failed to uninstall e2e"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t uninstall_e2e_firefox_internal<Os2Type<OS_WIN>>() {
  qDebug() << "quiting firefox";
  QString cmd("taskkill");
  QStringList args;
  args << "/F" << "/IM" << "firefox.exe" << "/T";
  system_call_res_t res =
      CSystemCallWrapper::ssystem(cmd, args, true, true, 10000);
  if(res.res != SCWE_SUCCESS || (res.exit_code != 0 && res.exit_code != 128)){
      qCritical() << "failed to close firefox"
                  << "exit code" << res.exit_code
                  << "output: " << res.out;
      return SCWE_CREATE_PROCESS;
  }

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path +=
      QString("\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%1\\extensions\\%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  QFile ext_file(ext_path);
  if (!ext_file.remove()) {
    qCritical() << "Failed to uninstall e2e"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::uninstall_e2e_firefox() {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_e2e_firefox_internal<Os2Type<CURRENT_OS>>();
  installer_is_busy.unlock();
  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::install_e2e_safari(const QString &dir, const QString &file_name) {
  QString cmd = "open";
  QStringList args;
  args << dir + QDir::separator() + file_name;
  qDebug() << "insalling e2e on safari"
           << "cmd:" << cmd
           << "args:" << args;
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 60000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to start e2e installation process"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  return SCWE_SUCCESS;
}

bool subutai_e2e_safari_exists(QString &safariextz_name) {
  QStringList lst =
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (lst.empty()) {
    qCritical() << "Unable to get standard home directory.";
    return SCWE_CREATE_PROCESS;
  }

  QString plist_path = *lst.begin() + "/Library/Safari/Extensions/";
  QString plist_original = plist_path + "Extensions.plist";
  if (!QFile::exists(plist_original)) {
    qCritical() << "cannot find" << plist_original;
    return SCWE_CREATE_PROCESS;
  }

  QString plist_copy = plist_path + "Extensions-tmp-copy.plist";
  if (QFile::exists(plist_copy)) {
    QFile::remove(plist_copy);
  }

  if (!QFile::copy(plist_original, plist_copy)) {
    qCritical() << "Failed to copy " << plist_original;
    return SCWE_CREATE_PROCESS;
  }

  QString cmd("/usr/bin/plutil");
  QStringList args;
  args << "-convert"
       << "xml1"
       << plist_copy;
  qDebug() << "Launching plutil to convert plist file to text file"
           << "cmd:" << cmd
           << "args:" << args;

  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to convert plist file"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    QFile::remove(plist_copy);
    return SCWE_CREATE_PROCESS;
  }

  QFile plist(plist_copy);
  if (!plist.open(QIODevice::ReadOnly)) {
    qCritical() << "Failed to open plist copy" << plist_copy;
    QFile::remove(plist_copy);
    return SCWE_CREATE_PROCESS;
  }

  QTextStream stream(&plist);
  QString str = stream.readAll();

  int id = str.indexOf("Installed Extensions");
  if (id != -1) {
    id = str.indexOf("Subutai E2E", id);
    if (id != -1) {
      int idr = str.indexOf("<", id);
      safariextz_name = str.mid(id, idr - id);
      QFile::remove(plist_copy);
      return true;
    }
  }

  QFile::remove(plist_copy);
  return false;
}

system_call_wrapper_error_t CSystemCallWrapper::subutai_e2e_safari_version(QString &version) {
  version = "undefined";

  QString safariextz_name;

  if (!subutai_e2e_safari_exists(safariextz_name)) {
    return SCWE_SUCCESS;
  }

  QStringList lst =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (lst.empty()) {
    qCritical() << "Unable to get standard home directory.";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = *lst.begin() + "/Library/Safari/Extensions/" + safariextz_name;
  if (!QFile::exists(ext_path)) {
    qCritical() << "Can't find safari e2e extension:" << ext_path;
    return SCWE_CREATE_PROCESS;
  }

  QString ext_ext = *lst.begin() + "/library/Safari/Extensions/e2e-plugin.safariextension";
  QDir ext_ext_dir(ext_ext);
  if (ext_ext_dir.exists()) {
    ext_ext_dir.removeRecursively();
  }

  QString cmd("xar");
  QStringList args;
  args << "-xf"
       << ext_path
       << "-C"
       << *lst.begin() + "/Library/Safari/Extensions/";
  qDebug() << "extracting e2e data from archive"
           << "cmd:" << cmd
           << "args:" << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to extract data"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  QString plist_path = *lst.begin() + "/Library/Safari/Extensions/e2e-plugin.safariextension/Info.plist";

  cmd = "/usr/bin/plutil";
  args.clear();
  args << "-convert"
       << "xml1"
       << plist_path;
  qDebug() << "Launching plutil to convert plist file to text file"
           << "cmd:" << cmd
           << "args:" << args;

  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) {
    qCritical() << "Failed to convert plist file"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }

  QFile plist(plist_path);

  if (!plist.open(QIODevice::ReadWrite)) {
    qCritical() << "Failed to open " << plist_path;
    return SCWE_CREATE_PROCESS;
  }

  QTextStream stream(&plist);
  QString str = stream.readAll();
  int id = str.indexOf("CFBundleShortVersionString");

  if (id != -1) {
    int idl = str.indexOf(QRegularExpression("[0-9.]"), id);
    if (idl != -1) {
      int idr = str.indexOf("<", idl);
      if (idr != -1) {
        version = str.mid(idl, idr - idl);
      }
    }
  }

  ext_ext_dir.removeRecursively();

  return SCWE_SUCCESS;
}

//Safari e2e extension name: "Subutai E2E Plugin.safariextz"

system_call_wrapper_error_t CSystemCallWrapper::uninstall_e2e_safari() {
  QString safariextz_name;
  if (!subutai_e2e_safari_exists(safariextz_name)) {
    qCritical() << "Subutai E2E not found";
    return SCWE_CREATE_PROCESS;
  }

  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << "tell application \"Safari\" to quit";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0){
    qCritical() << "Failed to close Safari"
                << "Exit code: " << res.exit_code
                << "Output: " << res.out;
    return SCWE_CREATE_PROCESS;
  }

  QStringList lst =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (lst.empty()) {
    qCritical() << "Failed to get standard home location.";
    return SCWE_CREATE_PROCESS;
  }
  QString path = *lst.begin() + "/Library/Safari/Extensions/" + safariextz_name;
  QFile ext(path);
  if (!ext.exists()) {
    qCritical() << "extension doesn't exist:" << path;
    return SCWE_CREATE_PROCESS;
  }
  if (!ext.remove()) {
    qCritical() << "Failed to remove extension:" << path;
    return SCWE_CREATE_PROCESS;
  }
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

system_call_wrapper_install_t CSystemCallWrapper::uninstall_e2e() {
  QString current_browser = CSettingsManager::Instance().default_browser();
  QString version;
  system_call_wrapper_install_t res;

  if (current_browser == "Chrome") {
    res.res = uninstall_e2e_chrome();
  } else if (current_browser == "Firefox") {
    res.res = uninstall_e2e_firefox();
  } else if (current_browser == "Safari") {
    res.res = uninstall_e2e_safari();
  }

  if (res.res == SCWE_SUCCESS && current_browser == "Chrome") {
    chrome_last_session();
    QTime dieTime = QTime::currentTime().addSecs(2);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
    subutai_e2e_version(version);
    res.version = version;
  } else if (res.res == SCWE_SUCCESS) {
    subutai_e2e_version(version);
    res.version = version;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_install_t CSystemCallWrapper::install_subutai_box(const QString &dir, const QString &file_name){
    installer_is_busy.lock();
    QString version;
    system_call_wrapper_install_t res;
    QString subutai_box = subutai_box_name();
    QString subutai_provider = VagrantProvider::Instance()->CurrentVal();
    system_call_wrapper_error_t res_t = vagrant_add_box(subutai_box, subutai_provider, dir + QDir::separator() + file_name);
    if(res_t == SCWE_SUCCESS) {
        QStringList lst_home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if(lst_home.isEmpty()){
            qCritical() << "Failed to get home directory";
            installer_is_busy.unlock();
            res.res = SCWE_CREATE_PROCESS;
            return res;
        }
        QDir boxes_path(lst_home[0] + QDir::separator() + ".vagrant.d" + QDir::separator() + "boxes");
        if(!boxes_path.exists()){
            qCritical() << "Vagrant is not installed correctly";
            installer_is_busy.unlock();
            res.res = SCWE_CREATE_PROCESS;
            return res;
        }
        QStringList parsed_box = subutai_box.split("/");
        QDir new_box_path(boxes_path.absolutePath() + QDir::separator() +
                          parsed_box[0] + "-VAGRANTSLASH-" + parsed_box[1] +
                          QDir::separator() + "0" + QDir::separator() + subutai_provider);
        if(!new_box_path.exists()){
            qCritical() << "vagrant box is not correctly installed";
            installer_is_busy.unlock();
            res.res = SCWE_CREATE_PROCESS;
            return res;
        }
        QString cloud_version = CRestWorker::Instance()->get_vagrant_box_cloud_version(subutai_box, subutai_provider);
        if(cloud_version == "undefined"){
            qCritical() << "couldn't get box cloud version";
            installer_is_busy.unlock();
            res.res = SCWE_CREATE_PROCESS;
            return res;
        }
        QDir cloud_box_path(boxes_path.absolutePath() + QDir::separator() +
                          parsed_box[0] + "-VAGRANTSLASH-" + parsed_box[1] +
                          QDir::separator() + cloud_version);
        if(!cloud_box_path.exists()){
            if(!cloud_box_path.mkdir(cloud_box_path.absolutePath())){
                qCritical() << "Failed to create directory for the new installed box";
                installer_is_busy.unlock();
                res.res = SCWE_CREATE_PROCESS;
                return res;
            }
        }
        QDir move;
        if(!move.rename(new_box_path.absolutePath(), cloud_box_path.absolutePath() + QDir::separator() + subutai_provider)){
            qCritical() << "failed to move" << new_box_path.absolutePath()
                        << "to" << cloud_box_path.absolutePath();
            res.res = SCWE_CREATE_PROCESS;
            return res;
        }
        QString metada_url(boxes_path.absolutePath() + QDir::separator() +
                           parsed_box[0] + "-VAGRANTSLASH-" + parsed_box[1] + QDir::separator() + "metadata_url");
        QFileInfo file_info(metada_url);
        if (!file_info.exists()) {
            QFile file(metada_url);
            if (file.open(QIODevice::ReadWrite)) {
              QTextStream stream(&file);
              stream << "https://vagrantcloud.com/" << parsed_box[0] << "/" << parsed_box[1];
            } else {
                qCritical() << "failed to created metadata_url file:" << file_info.absoluteFilePath();
                installer_is_busy.unlock();
                res.res = SCWE_CREATE_PROCESS;
                return res;
            }
        }
    }
    res.res = res_t;

    if (res.res == SCWE_SUCCESS) {
      vagrant_latest_box_version(subutai_box, subutai_provider, version);
      res.version = version;
    }

    installer_is_busy.unlock();
    return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_install_t CSystemCallWrapper::install_xquartz(const QString &dir,
                                                                const QString &file_name) {
  installer_is_busy.lock();
  QString cmd("osascript");
  QString version;
  QStringList args;
  QString file_path  = dir + QDir::separator() + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  system_call_wrapper_install_t res;
  system_call_res_t res_t = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  qDebug() << "xquartz installation has finished"
           << "exit code: " << res_t.exit_code
           << "result code: " << res_t.res
           << "output: " << res_t.out;

  if (res_t.exit_code != 0) {
    res_t.res = SCWE_CREATE_PROCESS;
  }
  res.res = res_t.res;

  if (res.res == SCWE_SUCCESS) {
    xquartz_version(version);
    res.version = version;
  }

  installer_is_busy.unlock();
  return res;
}
system_call_wrapper_install_t CSystemCallWrapper::uninstall_xquartz() {
  qDebug("Uninstalling xquartz");
  // sudo launchctl unload /Library/LaunchDaemons/io.subutai.p2p.daemon.plist
  QString cmd("osascript");
  QStringList args;
  QString version;
  system_call_wrapper_install_t res_install;

  args << "-e"
       << QString("do shell script \"launchctl unload /Library/LaunchDaemons/org.macosforge.xquartz.privileged_startx.plist; "
                  "rm -rf %1 \" with administrator privileges").arg(CSettingsManager::Instance().xquartz_path());
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  if (res.exit_code != 0) res.res = SCWE_CREATE_PROCESS;

  res_install.res = res.res;

  if (res_install.res == SCWE_SUCCESS) {
    xquartz_version(version);
    res_install.version = version;
  }

  return res_install;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::install_libssl(){
    QString pkexec_path;
    system_call_wrapper_error_t scr = CSystemCallWrapper::which("pkexec", pkexec_path);
    if (scr != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Unable to find pkexec command. You may reinstall the Control Center or reinstall the PolicyKit.");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
      return SCWE_WHICH_CALL_FAILED;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
        QString err_msg = QObject::tr("Unable to find sh command. Make sure that the command exists on your system or reinstall Linux.");
        qCritical() << err_msg;
        CNotificationObserver::Error(err_msg, DlgNotification::N_NO_ACTION);
        return SCWE_WHICH_CALL_FAILED;
    }

    QStringList lst_temp = QStandardPaths::standardLocations(QStandardPaths::TempLocation);

    if (lst_temp.empty()) {
      QString err_msg = QObject::tr("Unable to get the standard temporary location. Verify that your file system is setup correctly and fix any issues.");
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QString tmpFilePath =
        lst_temp[0] + QDir::separator() + "install_libssl1.0-dev.sh";

    qDebug() << tmpFilePath;

    QFile tmpFile(tmpFilePath);
    if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
      QString err_msg = QObject::tr("Couldn't create install script temp file. %1")
                        .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    QByteArray install_script = QString(
                                    "#!/bin/bash\n"
                                    "apt-get install -y libssl1.0-dev")
                                    .toUtf8();

    if (tmpFile.write(install_script) != install_script.size()) {
      QString err_msg = QObject::tr("Couldn't write install script to temp file")
                               .arg(tmpFile.errorString());
      qCritical() << err_msg;
      CNotificationObserver::Info(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    tmpFile.close();  // save

    if (!QFile::setPermissions(
            tmpFilePath,
            QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
      QString err_msg = QObject::tr("Couldn't set exe permission to reload script file");
      qCritical() << err_msg;
      CNotificationObserver::Error(err_msg, DlgNotification::N_SETTINGS);
      return SCWE_CREATE_PROCESS;
    }

    system_call_res_t cr2;
    QStringList args2;
    args2 << sh_path << tmpFilePath;
    cr2 = CSystemCallWrapper::ssystem_th(pkexec_path, args2, true, true, 97);
    qDebug()
            <<"libssl1.0 installation finished"
            <<"error code:"<<cr2.exit_code
            <<"output: "<<cr2.out
            <<"result: "<<cr2.res;
    tmpFile.remove();
    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
      return SCWE_CREATE_PROCESS;

    return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSystemCallWrapper::run_linux_script(QStringList args){
    UNUSED_ARG(args);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t run_sshpass_in_terminal_internal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &pass);


template<>
system_call_wrapper_error_t run_sshpass_in_terminal_internal<Os2Type<OS_WIN> >(const QString &user,
                                                                           const QString &ip,
                                                                           const QString &port,
                                                                           const QString &pass) {

  UNUSED_ARG(user);
  UNUSED_ARG(ip);
  UNUSED_ARG(port);
  UNUSED_ARG(pass);

#ifdef RT_OS_WINDOWS
  QString str_command = QString("\"%1\" %2@%3 -p %4")
                            .arg(CSettingsManager::Instance().ssh_path())
                            .arg(user)
                            .arg(ip)
                            .arg(port);

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }
  cmd = CSettingsManager::Instance().terminal_cmd();

  QString known_hosts = CSettingsManager::Instance().ssh_keys_storage() +
                        QDir::separator() + "known_hosts";
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};
  QString cmd_args =
      QString("\"%1\" /k \"%2 -o UserKnownHostsFile=%3\"").arg(cmd).arg(str_command).arg(known_hosts);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(nullptr, cmd_args_lpwstr, nullptr, nullptr, FALSE, 0, nullptr,
                          nullptr, &si, &pi);
  if (!cp) {
    qCritical(
        "Failed to create process %s. Err : %d", cmd.toStdString().c_str(),
        GetLastError());
    return SCWE_SSH_LAUNCH_FAILED;
  }
#endif

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t run_sshpass_in_terminal_internal<Os2Type<OS_LINUX> >(const QString &user,
                                                                           const QString &ip,
                                                                           const QString &port,
                                                                           const QString &pass) {
  UNUSED_ARG(pass);
  QString str_command = QString("%1 %2@%3 -p %4")
                            .arg(CSettingsManager::Instance().ssh_path())
                            .arg(user)
                            .arg(ip)
                            .arg(port);

  QString cmd;
  QFile cmd_file(CSettingsManager::Instance().terminal_cmd());
  if (!cmd_file.exists()) {
    system_call_wrapper_error_t tmp_res;
    if ((tmp_res = CSystemCallWrapper::which(CSettingsManager::Instance().terminal_cmd(), cmd)) !=
        SCWE_SUCCESS) {
      return tmp_res;
    }
  }

  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args = CSettingsManager::Instance().terminal_arg().split(
                       QRegularExpression("\\s"));
  args << QString("%1;bash").arg(str_command);
  return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS
                                            : SCWE_SSH_LAUNCH_FAILED;
}

template<>
system_call_wrapper_error_t run_sshpass_in_terminal_internal<Os2Type<OS_MAC> >(const QString &user,
                                                                           const QString &ip,
                                                                           const QString &port,
                                                                           const QString &pass) {
  UNUSED_ARG(pass);
  QString str_command = QString("%1 %2@%3 -p %4")
                            .arg(CSettingsManager::Instance().ssh_path())
                            .arg(user)
                            .arg(ip)
                            .arg(port);
  QString cmd;
  cmd = CSettingsManager::Instance().terminal_cmd();
  QStringList args;
  args << QString("-e");
  qInfo("Launch command : %s",
                                       str_command.toStdString().c_str());

  args << QString("Tell application \"%1\" to %2 \"%3\"")
              .arg(cmd, CSettingsManager::Instance().terminal_arg(), str_command);
  return QProcess::startDetached(QString("osascript"), args) ? SCWE_SUCCESS
                                            : SCWE_SSH_LAUNCH_FAILED;

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::run_sshpass_in_terminal(
    const QString &user, const QString &ip, const QString &port,
    const QString &pass) {
  return run_sshpass_in_terminal_internal<Os2Type<CURRENT_OS> >(user, ip, port, pass);
}



////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t run_x2goclient_session_internal(const QString &session_id);

template<>
system_call_wrapper_error_t run_x2goclient_session_internal<Os2Type<OS_MAC> >(
        const QString &session_id) {
    QString session_file_path = X2GoClient::x2goclient_config_path();
    QString cmd = "open";
    QStringList lst_args;
    lst_args
        << "-a"
        << CSettingsManager::Instance().x2goclient()
        << "--args"
        << QString("--session-conf=%1").arg(session_file_path)
        << QString("--sessionid=%1").arg(session_id)
        << "--hide"
        << "--no-menu"
        << "--debug"
        << "--thinclient";

    return QProcess::startDetached(cmd, lst_args) ? SCWE_SUCCESS
                                              : SCWE_SSH_LAUNCH_FAILED;
}

template<>
system_call_wrapper_error_t run_x2goclient_session_internal<Os2Type<OS_WIN> >(
        const QString &session_id) {
    QString session_file_path = X2GoClient::x2goclient_config_path();
    QString cmd = CSettingsManager::Instance().x2goclient();
    QStringList lst_args;
    lst_args
        << QString("--session-conf=%1").arg(session_file_path)
        << QString("--sessionid=%1").arg(session_id)
        << "--hide"
        << "--no-menu"
        << "--debug"
        << "--thinclient";

    return QProcess::startDetached(cmd, lst_args) ? SCWE_SUCCESS
                                              : SCWE_SSH_LAUNCH_FAILED;
}

template<>
system_call_wrapper_error_t run_x2goclient_session_internal<Os2Type<OS_LINUX> >(
        const QString &session_id) {
    QString session_file_path = X2GoClient::x2goclient_config_path();
    QString cmd = CSettingsManager::Instance().x2goclient();
    QStringList lst_args;
    lst_args
        << QString("--session-conf=%1").arg(session_file_path)
        << QString("--sessionid=%1").arg(session_id)
        << "--hide"
        << "--no-menu"
        << "--debug"
        << "--thinclient";

    qDebug() << "X2GOCLIENT launch cmd: "
             << cmd
             << lst_args;

    return QProcess::startDetached(cmd, lst_args) ? SCWE_SUCCESS
                                              : SCWE_SSH_LAUNCH_FAILED;
}

system_call_wrapper_error_t CSystemCallWrapper::run_x2goclient_session(const QString &session_id) {
  return run_x2goclient_session_internal<Os2Type<CURRENT_OS> >(session_id);
}

////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::generate_ssh_key(
    const QString &comment, const QString &file_path) {
  QString cmd = CSettingsManager::Instance().ssh_keygen_cmd();
  QStringList lst_args;
  lst_args << "-t"
           << "rsa"
           << "-f" << file_path << "-C" << comment << "-N"
           << "";
  QStringList lst_out;
  system_call_res_t res = ssystem(cmd, lst_args, true, true);

  if (res.exit_code != 0 && res.res == SCWE_SUCCESS) {
    res.res = SCWE_CANT_GENERATE_SSH_KEY;
    qCritical(
        "Can't generate ssh-key %s. exit_code : %d",
        file_path.toStdString().c_str(), res.exit_code);
  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::remove_ssh_key(const QString &key_name) {
  QString storage = CSettingsManager::Instance().ssh_keys_storage();
  QFileInfo fi(storage);
  if (!fi.isDir()) {
    return SCWE_WRONG_FILE_NAME;
  } else if (!fi.isWritable()) {
    return SCWE_PERMISSION_DENIED;
  }

  QString public_key = storage + QDir::separator() + key_name;
  if (QFile::exists(public_key)) {
    QFile::remove(public_key);
  }

  QString private_key = storage + QDir::separator() + key_name.left(key_name.size() - 4);
  if (QFile::exists(private_key)) {
    QFile::remove(private_key);
  }

  qDebug() << "SSH-key removal has been completed:"
           << public_key << (QFile::exists(public_key) ? "exists." : "has been removed.")
           << private_key << (QFile::exists(private_key) ? "exists." : "has been removed.");

  return (!QFile::exists(public_key) &&
          !QFile::exists(private_key)) ? SCWE_SUCCESS : SCWE_CREATE_PROCESS;
}

////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::run_libssh2_command(
    const char *host, uint16_t port, const char *user, const char *pass,
    const char *cmd, int &exit_code, std::vector<std::string> &lst_output) {
  static const int default_timeout = 10;
  exit_code = CLibsshController::run_ssh_command_pass_auth(
      host, port, user, pass, cmd, default_timeout, lst_output);

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::is_peer_available(const QString &peer_fingerprint, int *exit_code) {
  static const int default_timeout = 10;
  *exit_code
      = CLibsshController::check_auth_pass(
          CSettingsManager::Instance().rh_host(peer_fingerprint).toStdString().c_str(),
          CSettingsManager::Instance().rh_port(peer_fingerprint),
          CSettingsManager::Instance().rh_user(peer_fingerprint).toStdString().c_str(),
          CSettingsManager::Instance().rh_pass(peer_fingerprint).toStdString().c_str(),
          default_timeout);
  return *exit_code == 0 ? SCWE_SUCCESS : SCWE_CANT_GET_RH_IP;
}

system_call_wrapper_error_t CSystemCallWrapper::is_rh_update_available(
    bool &available) {
  available = false;
  return SCWE_SUCCESS;
  int exit_code = 0;
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res = run_libssh2_command(
      CSettingsManager::Instance().rh_host().toStdString().c_str(),
      CSettingsManager::Instance().rh_port(),
      CSettingsManager::Instance().rh_user().toStdString().c_str(),
      CSettingsManager::Instance().rh_pass().toStdString().c_str(),
      QString("sudo %1 update rh -c")
          .arg(CSettingsManager::Instance().subutai_cmd())
          .toStdString()
          .c_str(),
      exit_code, lst_out);
  if (res != SCWE_SUCCESS) return res;
  available = exit_code == 0;
  return SCWE_SUCCESS;  // doesn't matter I guess.
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t
CSystemCallWrapper::is_rh_management_update_available(bool &available) {
  available = false;
  return SCWE_SUCCESS;
  int exit_code = 0;
  std::vector<std::string> lst_out;

  system_call_wrapper_error_t res = run_libssh2_command(
      CSettingsManager::Instance().rh_host().toStdString().c_str(),
      CSettingsManager::Instance().rh_port(),
      CSettingsManager::Instance().rh_user().toStdString().c_str(),
      CSettingsManager::Instance().rh_pass().toStdString().c_str(),
      QString("sudo %1 update management -c")
          .arg(CSettingsManager::Instance().subutai_cmd())
          .toStdString()
          .c_str(),
      exit_code, lst_out);
  if (res != SCWE_SUCCESS) {
    qCritical(
        "is_rh_management_update_available failed with code %d", res);
    return res;
  }
  available = exit_code == 0;
  return SCWE_SUCCESS;  // doesn't matter I guess.
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::run_rh_updater(const char *host,
                                                               uint16_t port,
                                                               const char *user,
                                                               const char *pass,
                                                               int &exit_code) {
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass,
                          QString("sudo %1 update rh")
                              .arg(CSettingsManager::Instance().subutai_cmd())
                              .toStdString()
                              .c_str(),
                          exit_code, lst_out);
  return res;
}

system_call_wrapper_error_t CSystemCallWrapper::run_rh_management_updater(
    const char *host, uint16_t port, const char *user, const char *pass,
    int &exit_code) {
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res =
      run_libssh2_command(host, port, user, pass,
                          QString("sudo %1 update management")
                              .arg(CSettingsManager::Instance().subutai_cmd())
                              .toStdString()
                              .c_str(),
                          exit_code, lst_out);
  return res;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::get_rh_ip_via_libssh2(
    const char *host, uint16_t port, const char *user, const char *pass,
    int &exit_code, std::string &ip) {
  system_call_wrapper_error_t res;
  std::vector<std::string> lst_out;
  QString rh_ip_cmd = QString("sudo %1 info ipaddr")
                          .arg(CSettingsManager::Instance().subutai_cmd());
  res =
      run_libssh2_command(host, port, user, pass,
                          rh_ip_cmd.toStdString().c_str(), exit_code, lst_out);

  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    QHostAddress addr(lst_out[0].c_str());
    if (!addr.isNull()) {
      ip = addr.toString().toStdString();
      return SCWE_SUCCESS;
    }
  }

  return SCWE_CANT_GET_RH_IP;
}
////////////////////////////////////////////////////////////////////////////

QString CSystemCallWrapper::rh_version() {
  int exit_code;
  std::string version = "undefined";
  return QString::fromStdString(version);
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res = run_libssh2_command(
      CSettingsManager::Instance().rh_host().toStdString().c_str(),
      CSettingsManager::Instance().rh_port(),
      CSettingsManager::Instance().rh_user().toStdString().c_str(),
      CSettingsManager::Instance().rh_pass().toStdString().c_str(),
      QString("sudo %1 -v")
          .arg(CSettingsManager::Instance().subutai_cmd())
          .toStdString()
          .c_str(),
      exit_code, lst_out);
  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty())
    version = lst_out[0];

  size_t index;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");

  return QString::fromStdString(version);
}
////////////////////////////////////////////////////////////////////////////

QString CSystemCallWrapper::rhm_version() {
  int exit_code;
  std::string version = "undefined";
  return QString::fromStdString(version);
  std::vector<std::string> lst_out;
  system_call_wrapper_error_t res = run_libssh2_command(
      CSettingsManager::Instance().rh_host().toStdString().c_str(),
      CSettingsManager::Instance().rh_port(),
      CSettingsManager::Instance().rh_user().toStdString().c_str(),
      CSettingsManager::Instance().rh_pass().toStdString().c_str(),
      QString("sudo %1 attach management grep git.build.version "
              "/opt/subutai-mng/etc/git.properties")
          .arg(CSettingsManager::Instance().subutai_cmd())
          .toStdString()
          .c_str(),
      exit_code, lst_out);
  if (res == SCWE_SUCCESS && exit_code == 0 && !lst_out.empty()) {
    for (auto str = lst_out.begin(); str != lst_out.end(); ++str) {
      if (str->find("git.build.version") == std::string::npos) continue;
      version = str->substr(str->find("=") + 1);
    }
  }

  size_t index;
  if ((index = version.find('\n')) != std::string::npos)
    version.replace(index, 1, " ");

  return QString::fromStdString(version);
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::kvm_version(QString &version) {
  version = "undefined";
  QStringList args;

  args << "--version";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(
      QString("kvm"), args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    QRegExp reg("(\\d+\\.\\d+\\.\\d+)");

    if (reg.indexIn(ver, 0) != -1) {
      version = reg.cap(1);
    }
  }

  return res.res;
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::p2p_version(QString &version) {
  version = "undefined";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "-v";
  system_call_res_t res = ssystem_th(cmd, args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()){
    version = res.out[0];
  }
  else{
      res.res = SCWE_CREATE_PROCESS;
  }
  version = version.remove("p2p");
  version = version.remove("version");
  version = version.remove("  ");
  int id = version.indexOf("+");
  version = version.left(id);
  return res.res;
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t x2go_version_internal(QString &version);

template<>
system_call_wrapper_error_t x2go_version_internal <Os2Type <OS_LINUX> > (QString &version){
  QString cmd = "script";
  QStringList args;
  args << "-q"
       << "/dev/stdout"
       << "-c"
       << CSettingsManager::Instance().x2goclient() + " -v";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.res == SCWE_SUCCESS && res.exit_code == 0) {
    QRegExp pattern("([0-9]+[.])+([0-9]+)");
    for (auto s : res.out) {
      s = s.simplified();
      if (pattern.exactMatch(s)) {
        version = s;
        break;
      }
    }
  }
  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t x2go_version_internal <Os2Type <OS_MAC> > (QString &version){
  //mdls -name kMDItemVersion /Applications/x2goclient.app/
  version = "undefined";
  qDebug() << "getting x2go version";
  QString cmd = "mdls";
  QStringList args;
  args << "-name"
       << "kMDItemVersion"
       << "/Applications/x2goclient.app/";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.exit_code != 0 || res.out.length() != 1) {
    return SCWE_CREATE_PROCESS;
  }
  version = res.out[0].remove("kMDItemVersion = ").remove("\"");
  return res.res;
}

template<>
system_call_wrapper_error_t x2go_version_internal <Os2Type <OS_WIN> > (QString &version){
  UNUSED_ARG(version);
  QString cmd("REG");
  QStringList args;
  args
    << "QUERY"
    << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\x2goclient"
    << "/v"
    << "DisplayVersion";
  qDebug() << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()
      << "got x2go client version"
      << "exit code" << res.exit_code
      << "result code" << res.res
      << "output" << res.out;
  if (res.res == SCWE_SUCCESS &&
      res.exit_code == 0 && !res.out.empty()) {
    for (QString s : res.out) {
      s = s.trimmed();
      if (s.isEmpty()) continue;
      QStringList buf = s.split(" ", QString::SkipEmptyParts);
      if (buf.size() == 3) {
        QStringList second_buf = buf[2].split("-", QString::SkipEmptyParts);
        if (!second_buf.isEmpty()) {
          version = second_buf[0];
        }
        break;
      }
   }
  }
  return SCWE_SUCCESS;
}
system_call_wrapper_error_t CSystemCallWrapper::x2go_version(QString &version){
  version = "undefined";
  return x2go_version_internal <Os2Type <CURRENT_OS>>(version);
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t vagrant_version_internal(QString &version);

/********************/
template<>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_MAC_LIN> >(
    QString &version) {
  version = "undefined";
  QStringList args;
  args << "--version";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().vagrant_path(), args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    version = ver.remove(QRegularExpression("[a-zA-Z ]*"));
  }

  return res.res;
}
/********************/

template<>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return vagrant_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return vagrant_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_WIN> >(
    QString &version) {
  version = "undefined";
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;
  args
    << "-v";
  qDebug()<<args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"got vagrant version"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    version = ver.remove(QRegularExpression("[a-zA-Z ]*"));
  }
  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_version(
    QString &version) {
  return vagrant_version_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t oracle_virtualbox_version_internal(QString &version);

/********************/
template<>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_MAC_LIN> >(
    QString &version) {
  version = "undefined";
  QStringList args;
  args << "--version";

  QString path = CSettingsManager::Instance().oracle_virtualbox_path();
  QDir dir(path);
  dir.cdUp();
  path = dir.absolutePath();
  path += "/VBoxManage";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(path,
                                                         args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QRegExp pattern("([0-9]+[.])+[0-9]+[r]+[0-9]+");
    for (auto str: res.out) {
      QString ver = str.simplified();
      if (pattern.exactMatch(ver)) {
        version = ver.remove(QRegularExpression("([ ]*)?(r[0-9]*)?"));
        break;
      }
    }
  }

  return res.res;
}
/********************/

template<>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return oracle_virtualbox_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return oracle_virtualbox_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_WIN> >(
    QString &version) {
  version = "undefined";
  QString cmd = CSettingsManager::Instance().oracle_virtualbox_path();
  cmd.remove("VirtualBox.exe").append("VBoxManage.exe");
  QStringList args;
  args
    << "-v";
  qDebug()<<args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"got oracle virtualbox version"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QRegExp pattern("([0-9]+[.])+[0-9]+[r]+[0-9]+");
    for (auto str: res.out) {
      QString ver = str.simplified();
      if (pattern.exactMatch(ver)) {
        version = ver.remove(QRegularExpression("([ ]*)?(r[0-9]*)?"));
        break;
      }
    }
  }
  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::oracle_virtualbox_version(
    QString &version) {
  return oracle_virtualbox_version_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////
//  VMWARE VERSION
template <class OS>
system_call_wrapper_error_t vmware_version_internal(QString &version);

template <>
system_call_wrapper_error_t vmware_version_internal<Os2Type<OS_MAC> >(QString &version) {

  QString path = CSettingsManager::Instance().vmware_path();
   QDir dir(path);
   dir.cdUp(); dir.cdUp();
   path = dir.absolutePath();
   path += "/Info.plist";

   QFile info_plist(path);
   QString line;
   bool found = false;

   if (!info_plist.exists()) {
     version = "undefined";

     return SCWE_SUCCESS;
   }

   if (info_plist.open(QIODevice::ReadOnly | QIODevice::Text)) {
       QTextStream stream(&info_plist);
       while (!stream.atEnd()) {
         line = stream.readLine();
         line = line.simplified();
         line.remove(QRegExp("[<]([/]?[a-z]+)[>]"));
         if (found) {
             version = line;
             break;
         }
         else if (line == "CFBundleShortVersionString") {
             found = true;
         }
     }
     info_plist.close();
   }

   return SCWE_SUCCESS;
}

template <>
system_call_wrapper_error_t vmware_version_internal<Os2Type<OS_WIN> >(QString &version) {
  version = "undefined";
  QString cmd("REG");
  QStringList args;
  // REG QUERY "HKLM\SOFTWARE\WOW6432Node\VMware, Inc.\VMware Workstation" /v ProductVersion
  args << "QUERY"
       << "HKLM\\SOFTWARE\\WOW6432Node\\VMware, Inc.\\VMware Workstation"
       << "/v"
       << "ProductVersion";

  qDebug() << "Register query VMware: "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);

  qDebug() << "got VMware version"
           << " exit code: "
           << res.exit_code
           << " result code: "
           << res.res
           << " output: "
           << res.out;

  if (res.res == SCWE_SUCCESS &&
      res.exit_code == 0 && !res.out.empty()) {
    QRegExp pattern("ProductVersion*");
    pattern.setPatternSyntax(QRegExp::Wildcard);

    for (QString s : res.out) {
      s = s.trimmed();
      if (s.isEmpty()) continue;

      if (pattern.exactMatch(s)) {
        QStringList buf = s.split(" ", QString::SkipEmptyParts);
        if (buf.size() == 3) {
          version = buf[2];
          break;
        }
      }
    }
  }

  return SCWE_SUCCESS;
}

template <>
system_call_wrapper_error_t vmware_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  qDebug() << "VMware version";
  version = "undefined";
  QStringList args;
  args << "--version";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(
      QString("/usr/bin/vmware"), args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    version = ver.remove(QRegularExpression("build-\\d*"));
    version = version.remove(QRegularExpression("[a-zA-Z ]*"));

  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vmware_version(QString &version) {
  return vmware_version_internal<Os2Type<CURRENT_OS> >(version);
}

system_call_wrapper_error_t CSystemCallWrapper::parallels_version(QString &version) {
  QString path = CSettingsManager::Instance().parallels_path();
   QDir dir(path);
   dir.cdUp(); dir.cdUp();
   path = dir.absolutePath();
   path += "/version.plist";

   QFile info_plist(path);
   QString line;
   bool found = false;

   if (!info_plist.exists()) {
     qDebug() << "Couldn't find" << path;
     version = "undefined";

     return SCWE_SUCCESS;
   }

   if (info_plist.open(QIODevice::ReadOnly | QIODevice::Text)) {
       QTextStream stream(&info_plist);
       while (!stream.atEnd()) {
         line = stream.readLine();
         line = line.simplified();
         line.remove(QRegExp("[<]([/]?[a-z]+)[>]"));
         if (found) {
             version = line;
             break;
         }
         else if (line == "CFBundleShortVersionString") {
             found = true;
         }
     }
     info_plist.close();
   }

   qDebug() << "Got parallels version:" << version;
   return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////

//  VMWARE VMWARE VERSION
template <class OS>
system_call_wrapper_error_t vmware_utility_version_internal(QString &version);

template <>
system_call_wrapper_error_t vmware_utility_version_internal<Os2Type<OS_MAC> >(QString &version) {
  version = "undefined";
  QString cmd = "/opt/vagrant-vmware-desktop/bin/vagrant-vmware-utility";
  QStringList args;
  args << "-v";
  system_call_res_t res = CSystemCallWrapper::ssystem_f(cmd, args, true, true, 5000);

  qDebug() << "Got Vagrant VMware Utility version"
           << " exit code: "
           << res.exit_code
           << " res code: "
           << res.res
           << " output: "
           << res.out;

  if (res.res == SCWE_SUCCESS && res.exit_code == 0) {
    version = res.out[0];
  } else {
      res.res = SCWE_CREATE_PROCESS;
  }

  return res.res;
}

template <>
system_call_wrapper_error_t vmware_utility_version_internal<Os2Type<OS_WIN> >(QString &version) {
  version = "undefined";
  QString cmd("wmic");
  QStringList args;
  // wmic product where name="Vagrant VMware Utility" get version
  args << "product"
       << "where"
       << QString("name=\"%1\"").arg("Vagrant VMware Utility")
       << "get"
       << "version";

  qDebug() << "Vagrant VMware utility version command "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);

  qDebug() << "got Vagrant VMware utility version"
           << " exit code: "
           << res.exit_code
           << " result code: "
           << res.res
           << " output: "
           << res.out;

  if (res.res == SCWE_SUCCESS &&
      res.exit_code == 0 && !res.out.empty()) {

    for (QString s : res.out) {
      s = s.trimmed();
      if (s.isEmpty()) continue;

      s.remove(QRegExp("[<]([/]?[a-z]+)[>]"));
      version = s;
    }
  }

  return SCWE_SUCCESS;/*

  version = "undefined";

  QString path;
  QString cmd("REG");
  QStringList args;
  args << "QUERY"
       << "HKLM\\SYSTEM\\CurrentControlSet\\Services\\VagrantVMware";
  qDebug() << "REG QUERY started"
           << "args:" << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);

  qDebug() << "REG QUERY finished:"
           << "exit code:" << res.exit_code
           << "output:" << res.out;

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "REG QUERY failed.";
    return SCWE_CREATE_PROCESS;
  }

  version = "installed";

  return SCWE_SUCCESS;*/
}

template <>
system_call_wrapper_error_t vmware_utility_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  qDebug() << "Vagrant VMware Utility version";
  //  dpkg-query -W -f='${Version}\n' vagrant-vmware-utility

  version = "undefined";
  QStringList args;
  args << "-W"
       << "-f='${Version}\\n'"
       << "vagrant-vmware-utility";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(
      QString("dpkg-query"), args, true, true, 97);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    version = ver.remove(QRegExp(".*:"));
  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vmware_utility_version(QString &version) {
  //std::this_thread::sleep_for(std::chrono::milliseconds(20 * 1000));
  QString vmware_ver;
  vmware_version(vmware_ver);
  if (vmware_ver == "undefined") {
    version = "Install VMware first";
    return SCWE_SUCCESS;
  }
  return vmware_utility_version_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_install_t CSystemCallWrapper::install_hyperv() {
  QString version("undefined");
  system_call_wrapper_install_t res_ins;
  QString cmd("DISM");
  QStringList args;
  // DISM /Online /Enable-Feature /All /FeatureName:Microsoft-Hyper-V /NoRestart
  args << "/Online"
       << "/Enable-Feature"
       << "/All"
       << "/FeatureName:Microsoft-Hyper-V"
       << "/NoRestart";

  qDebug() << "HyperV enable command "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 1000 * 60 * 30); // timeout 30 minutes

  qDebug() << "Hyperv enable: "
           << " exit code: "
           << res.exit_code
           << " result code: "
           << res.res
           << " output: "
           << res.out;

  if (res.res == SCWE_SUCCESS && !res.out.empty()) {
    version = "Enabled";
  }

  res_ins.res = res.res;
  res_ins.version = version;

  return res_ins;
}

system_call_wrapper_install_t CSystemCallWrapper::uninstall_hyperv() {
  QString version("undefined");
  system_call_wrapper_install_t res_ins;
  QString cmd("DISM");
  QStringList args;
  // DISM /Online /Disable-Feature:Microsoft-Hyper-V-All /NoRestart
  args << "/Online"
       << "/Disable-Feature:Microsoft-Hyper-V-All"
       << "/NoRestart";

  qDebug() << "HyperV disable command "
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 1000 * 60 * 30); // timeout 30 minutes

  qDebug() << "Hyperv disable: "
           << " exit code: "
           << res.exit_code
           << " result code: "
           << res.res
           << " output: "
           << res.out;

  res_ins.res = res.res;
  res_ins.version = version;

  return res_ins;
}

system_call_wrapper_error_t CSystemCallWrapper::hyperv_version(QString &version) {
  version = "undefined";
  QString cmd("powershell.exe");
  QString empty;

  system_call_wrapper_error_t cr;

  if ((cr = CSystemCallWrapper::which(cmd, empty)) != SCWE_SUCCESS) {
    qDebug() << "powershell command not found: check hyperv enabled";
  }

  QStringList args; //  powershell.exe -NoLogo -NoProfile -NonInteractive -ExecutionPolicy  Bypass -Command { $hyperv = Get-WindowsOptionalFeature -FeatureName Microsoft-Hyper-V-All -Online; echo $hyperv.State; }
  args << "-NoLogo"
       << "-NoProfile"
       << "-NonInteractive"
       << "-ExecutionPolicy"
       << "Bypass"
       << "-Command"
       << "Get-WindowsOptionalFeature -FeatureName Microsoft-Hyper-V-All -Online";

  qDebug() << "Hyper-V version command: "
           << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 1000 * 60);

  qDebug() << "Hyper-v version command finished: "
           << "exit code: "
           << res.exit_code
           << " output: "
           << res.out
           << " res: "
           << res.res;

  if (res.res == SCWE_SUCCESS &&
        res.exit_code == 0 && !res.out.empty()) {
    for (QString line : res.out) {
      if (line.contains("State")) {
        line.remove("State");
        line.remove(":");
        version = line.trimmed();

        if (version == "Disabled")
          version = "undefined";
        break;
      }
    }
   }

  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t subutai_e2e_version_internal(QString &version);

template<class OS>
system_call_wrapper_error_t subutai_e2e_chrome_version_internal(QString &version);

template<>
system_call_wrapper_error_t subutai_e2e_chrome_version_internal<Os2Type <OS_MAC_LIN> >(QString &version){
    QString current_browser = CSettingsManager::Instance().default_browser();
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (current_browser == "Chrome"){
        /*
         * to get version of chrome extension just check path
         * */
        version = "undefined";
        QString cmd("ls");
        QString ex_id = subutai_e2e_id(current_browser);
        QStringList args;
        QString chrome_profile = CSettingsManager::Instance().default_chrome_profile();
        args << QString("%1%3%4/Extensions/%2/").arg(homePath.first(), ex_id, default_chrome_extensions_path(), chrome_profile);
        system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
        if(res.res == SCWE_SUCCESS && res.exit_code == 0 && res.out.size() != 0){
            version = res.out[res.out.size() - 1];
            return SCWE_SUCCESS;
        }
    }
    return SCWE_CREATE_PROCESS;
}
template<>
system_call_wrapper_error_t subutai_e2e_chrome_version_internal<Os2Type <OS_LINUX> >(QString &version){
    return subutai_e2e_chrome_version_internal<Os2Type <OS_MAC_LIN> >(version);
}
template<>
system_call_wrapper_error_t subutai_e2e_chrome_version_internal<Os2Type <OS_MAC> >(QString &version){
    return subutai_e2e_chrome_version_internal<Os2Type <OS_MAC_LIN> >(version);
}
template<>
system_call_wrapper_error_t subutai_e2e_chrome_version_internal<Os2Type <OS_WIN> >(QString &version){
    QString current_browser = CSettingsManager::Instance().default_browser();
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    /*
     * to get version of chrome extension just check path
     * */
    version = "undefined";
    QString ex_id = subutai_e2e_id(current_browser);
    QString chrome_profile = CSettingsManager::Instance().default_chrome_profile();
    QString extension_path = QString("%1%3%4\\Extensions\\%2\\").arg(homePath.first().split(QDir::separator()).last(), ex_id, default_chrome_extensions_path(), chrome_profile);
    QDir extension_dir(extension_path);
    if(extension_dir.exists()){
        extension_dir.setFilter(QDir::Dirs);
        QStringList extension_entry_list = extension_dir.entryList();
        qDebug() << "extension entry" << extension_entry_list;
        if(!extension_entry_list.isEmpty()){
            version = extension_entry_list[extension_entry_list.size() - 1];
            return SCWE_SUCCESS;
        }
    }
    if(version == "undefined"){
        /* get the extension version from secondary profile
         * to get version of chrome extension just check path
         * */
        version = "undefined";
        QString ex_id = subutai_e2e_id(current_browser);
        QString chrome_profile = "Profile 3";
        QString extension_path = QString("%1%3%4\\Extensions\\%2\\").arg(homePath.first().split(QDir::separator()).last(), ex_id, default_chrome_extensions_path(), chrome_profile);
        QDir extension_dir(extension_path);
        if(extension_dir.exists()){
            extension_dir.setFilter(QDir::Dirs);
            QStringList extension_entry_list = extension_dir.entryList();
            qDebug() << "extension entry" << extension_entry_list;
            if(!extension_entry_list.isEmpty()){
                version = extension_entry_list[extension_entry_list.size() - 1];
                return SCWE_SUCCESS;
            }
        }
    }
    return SCWE_CREATE_PROCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::subutai_e2e_chrome_version(QString &version) {
  QString chrome_ver;
  CSystemCallWrapper::chrome_version(chrome_ver);

  if (chrome_ver == "undefined") {
    version = QObject::tr("No supported browser is available");
    return SCWE_SUCCESS;
  }

  return subutai_e2e_chrome_version_internal<Os2Type<CURRENT_OS>>(version);
}

template<class OS>
system_call_wrapper_error_t subutai_e2e_firefox_version_internal(QString &version);

template<>
system_call_wrapper_error_t subutai_e2e_firefox_version_internal<Os2Type<OS_LINUX>>(QString &version) {
  version = "undefined";

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path += QString("/.mozilla/firefox/%1/extensions/%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  qDebug() << "firefox e2e extension path" << ext_path;
  QFile ext_file(ext_path);
  if (ext_file.exists()) {
    QString addons_path = home_paths_list[0] +
        QString("/.mozilla/firefox/%1/extensions.json").arg(profile_folder);
    QFile addons_file(addons_path);
    if (!addons_file.open(QIODevice::ReadOnly)) {
      qCritical() << "Can't open extensions.json file";
      return SCWE_CREATE_PROCESS;
    }

    QJsonObject obj = QJsonDocument().fromJson(addons_file.readAll()).object();
    if (!obj.contains("addons")) {
      qCritical() << "no entry of subutai e2e in extensions.json";
      return SCWE_CREATE_PROCESS;
    }
    QJsonArray arr = obj["addons"].toArray();
    for (QJsonValue i: arr) {
      if (i.isObject()) {
        QJsonObject cur = i.toObject();
        if (cur.contains("id") && cur["id"].isString() &&
            cur["id"] == subutai_e2e_id("Firefox") + "@jetpack") {
          version = cur["version"].toString();
          break;
        }
      }
    }

    qDebug() << "got firefox subutai e2e version:" << version;
  } else {
    qDebug() << "firefox subutai e2e isn't installed";
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t subutai_e2e_firefox_version_internal<Os2Type<OS_MAC>>(QString &version) {
  version = "undefined";

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    qDebug() << "gege" << profiles.first[i] << profiles.second[i] << cur_profile;
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }
  qDebug() << "got profile folder:" << profile_folder;

  ext_path +=
      QString("/Library/Application Support/Firefox/Profiles/%1/extensions/%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  qDebug() << "firefox e2e extension path" << ext_path;
  QFile ext_file(ext_path);
  if (ext_file.exists()) {
    QString addons_path = home_paths_list[0] +
        QString("/Library/Application Support/Firefox/Profiles/%1/extensions.json")
        .arg(profile_folder);
    QFile addons_file(addons_path);
    if (!addons_file.open(QIODevice::ReadOnly)) {
      qCritical() << "Can't open extensions.json file";
      return SCWE_CREATE_PROCESS;
    }

    QJsonObject obj = QJsonDocument().fromJson(addons_file.readAll()).object();
    if (!obj.contains("addons")) {
      qCritical() << "no entry of subutai e2e in extensions.json";
      return SCWE_CREATE_PROCESS;
    }
    QJsonArray arr = obj["addons"].toArray();
    for (QJsonValue i: arr) {
      if (i.isObject()) {
        QJsonObject cur = i.toObject();
        if (cur.contains("id") && cur["id"].isString() &&
            cur["id"] == subutai_e2e_id("Firefox") + "@jetpack") {
          version = cur["version"].toString();
          break;
        }
      }
    }
  } else {
    qDebug() << "firefox subutai e2e isn't installed";
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t subutai_e2e_firefox_version_internal<Os2Type<OS_WIN>>(QString &version) {
  version = "undefined";

  QStringList home_paths_list =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (home_paths_list.empty()) {
    qCritical() << "Failed to get home directory";
    return SCWE_CREATE_PROCESS;
  }

  QString ext_path = home_paths_list[0];
  std::pair<QStringList, QStringList> profiles = firefox_profiles();
  QString cur_profile = CSettingsManager::Instance().default_firefox_profile();
  QString profile_folder = "";

  for (int i = 0; i < profiles.first.size(); i++) {
    if (profiles.first[i] == cur_profile) {
      profile_folder = profiles.second[i];
      break;
    }
  }

  ext_path +=
      QString("\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%1\\extensions\\%2@jetpack.xpi")
      .arg(profile_folder, subutai_e2e_id("Firefox"));
  qDebug() << "firefox e2e extension path" << ext_path;
  QFile ext_file(ext_path);
  if (ext_file.exists()) {
    QString addons_path = home_paths_list[0] +
        QString("\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%1\\extensions.json")
        .arg(profile_folder);
    QFile addons_file(addons_path);
    if (!addons_file.open(QIODevice::ReadOnly)) {
      qCritical() << "Can't open extensions.json file";
      return SCWE_CREATE_PROCESS;
    }
    QJsonObject obj = QJsonDocument().fromJson(addons_file.readAll()).object();
    if (!obj.contains("addons")) {
      qCritical() << "no entry of subutai e2e in extensions.json";
      return SCWE_CREATE_PROCESS;
    }

    QJsonArray arr = obj["addons"].toArray();
    for (QJsonValue i: arr) {
      if (i.isObject()) {
        QJsonObject cur = i.toObject();
        if (cur.contains("id") && cur["id"].isString() &&
            cur["id"] == subutai_e2e_id("Firefox") + "@jetpack") {
          version = cur["version"].toString();
          break;
        }
      }
    }
  } else {
    qDebug("firefox subutai e2e extension isn't installed");
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::subutai_e2e_firefox_version(QString &version) {
  QString firefox_ver;
  CSystemCallWrapper::firefox_version(firefox_ver);

  if (firefox_ver == "undefined") {
    version = QObject::tr("No supported browser is available");
    return SCWE_SUCCESS;
  }

  return subutai_e2e_firefox_version_internal<Os2Type<CURRENT_OS>>(version);
}

system_call_wrapper_error_t CSystemCallWrapper::subutai_e2e_version(QString &version){
  QString current_browser = CSettingsManager::Instance().default_browser();
  if (current_browser == "Chrome") {
    return CSystemCallWrapper::subutai_e2e_chrome_version(version);
  } else if (current_browser == "Firefox") {
    return CSystemCallWrapper::subutai_e2e_firefox_version(version);
  } else if (current_browser == "Safari") {
    return CSystemCallWrapper::subutai_e2e_safari_version(version);
  }
  version = "undefined";
  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::xquartz_version(QString &version){
  //mdls -name kMDItemVersion /Applications/Utilities/XQuartz.app/
  version = "undefined";
  qDebug() << "getting xquartz version";
  QString cmd = "mdls";
  QStringList args;
  args << "-name"
       << "kMDItemVersion"
       << "/Applications/Utilities/XQuartz.app/";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);
  if (res.exit_code != 0 || res.out.length() != 1) {
    return SCWE_CREATE_PROCESS;
  }
  version = res.out[0].remove("kMDItemVersion = ").remove("\"");
  return res.res;
}
////////////////////////////////////////////////////////////////////////////
//* get vagrant plugin list and find there required plugin version *//
system_call_wrapper_error_t CSystemCallWrapper::vagrant_plugin_version(QString &version, QString vagrant_plugin) {
  qDebug() << QString("Getting version of %1 plugin").arg(vagrant_plugin);

  vagrant_version(version);

  if (version == "undefined"){
    version = QObject::tr("Install Vagrant first");
    return SCWE_CREATE_PROCESS;
  }

  version = "undefined";
  std::vector<std::pair<QString, QString> > plugin_list;
  CSystemCallWrapper::vagrant_plugins_list(plugin_list);

  auto it = std::find_if(plugin_list.begin(), plugin_list.end(), [vagrant_plugin](const std::pair<QString, QString>& plugin) {
    return vagrant_plugin == plugin.first;
  });

  if (it == plugin_list.end()) {
    return SCWE_CREATE_PROCESS;
  } else {
    version = it->second;
    if(version.size() >= 2) { //remove ( ) in the beginning and in the end
      version.remove(0, 1);

      if (version[version.size() - 1] == '\r' || version[version.size() - 1] == '\t' || version[version.size() - 1] == '\n') {
        version.remove(version.size() - 1, 1);
      }
          version.remove(version.size() - 1, 1);
    }
  }

  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::p2p_status(QString &status) {
  status = "";
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "status";
  system_call_res_t res = ssystem_th(cmd, args, true, false, 5000);
  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    for (auto i = res.out.begin(); i != res.out.end(); ++i) status += *i;
  } else {
    status = "undefined";
  }
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::p2p_daemon_check() {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show";
  system_call_res_t cr = ssystem_th(cmd, args, true, true, 5000);
  return !(cr.exit_code || cr.res);
}
////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::x2goclient_check() {
#ifndef RT_OS_DARWIN
  return CCommons::IsApplicationLaunchable(CSettingsManager::Instance().x2goclient());
#endif
  return CCommons::IsTerminalLaunchable(CSettingsManager::Instance().x2goclient());
}

////////////////////////////////////////////////////////////////////////////
/// \brief CSystemCallWrapper::which
/// \param prog
/// \param path
/// \return
///
system_call_wrapper_error_t CSystemCallWrapper::which(const QString &prog,
                                                      QString &path) {
  static int success_ec = 0;
  QString cmd(which_cmd());
  QStringList args;
  args << prog;
  system_call_res_t res = ssystem_th(cmd, args, true, true, 5000);
  qDebug()<<"requested which for"<<prog<<"exit code:"<<res.exit_code
         <<"result:"<<res.res<<"output:"<<res.out;
  if(res.exit_code == 1 && cmd == "/usr/bin/which"){ //search in local bin (mac specific)
      qDebug()<<"search again in local bin";
      args.clear();
      args << "/usr/local/bin/" + prog;
      res = ssystem_th(cmd, args, true, true, 5000);
      qDebug() << "finished which for"
               << args
               << "result : " << res.res
               << "exit code :" << res.exit_code
               << "output : " << res.out;
  }
  if (res.res != SCWE_SUCCESS) return res.res;

  if (res.exit_code == success_ec && !res.out.empty()) {
    path = res.out[0];
    path.replace("\n", "\0");
    path.replace("\r", "\0");
    return SCWE_SUCCESS;
  }

  return SCWE_WHICH_CALL_FAILED;
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::open(const QString &prog) {
  static int success_ec = 0;
  QString cmd("open");
  QStringList args;
  args << QString("-Ra");
  args << prog;
  system_call_res_t res = ssystem_th(cmd, args, false, false, 5000);
  if (res.res != SCWE_SUCCESS) return res.res;

  if (res.exit_code == success_ec) {
    return SCWE_SUCCESS;
  }

  return SCWE_WHICH_CALL_FAILED;
}

////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::is_desktop_peer() {
#ifndef RT_OS_LINUX
  return false;
#endif
  QString cmd("subutai");
  QString empty;
  return which(cmd, empty) == SCWE_SUCCESS;
}


system_call_wrapper_error_t CSystemCallWrapper::local_containers_list(QStringList &list) {
  qDebug() << "Getting list of local containers";
  list.clear();
  static QString lxc_path("/var/lib/lxc");
  QDir directory(lxc_path);
  QString tmp;

  if (!directory.exists()) {
    qCritical() << "container directory not exist: "
                << lxc_path;
    return SCWE_CREATE_PROCESS;
  }

  for (QFileInfo info : directory.entryInfoList()) {
    QString tmp = info.fileName();
    qDebug() << "local container foreach: " << info.fileName();
    if (tmp.contains("Container"))  {
      qDebug() << "found local container: "
               << info.fileName();
      list << info.fileName();
    }
  }

  qDebug() << "List of local containers:" << list;
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t chrome_version_internal(QString &version);

/********************/
template<>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_MAC_LIN> >(
    QString &version) {
  version = "undefined";
  QStringList args;
  args << "--version";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(
      CSettingsManager::Instance().chrome_path(), args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    QString ver = res.out[0];
    version = ver.remove(QRegularExpression("[a-zA-Z ]*"));
  }

  int index;
  if ((index = version.indexOf('\n')) != -1) version.replace(index, 1, " ");
  return res.res;
}
/********************/

template<>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template<>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_WIN> >(
    QString &version) {
  version = "undefined";
  QString cmd("REG");
  QStringList args;
  args
    << "QUERY"
    << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Google Chrome"
    << "/v"
    << "DisplayVersion";
  qDebug()<<args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"got chrome version"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
      for (QString s : res.out){
          s = s.trimmed();
          if(s.isEmpty()) continue;
          QStringList buf = s.split(" ", QString::SkipEmptyParts);
          if(buf.size() == 3){
              version = buf[2];
              break;
          }
      }
  }
  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::chrome_version(
    QString &version) {
  return chrome_version_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
system_call_wrapper_error_t firefox_version_internal(QString &version);

template<>
system_call_wrapper_error_t firefox_version_internal<Os2Type<OS_MAC_LIN>>(QString &version) {
  version = "undefined";

  QString cmd = CSettingsManager::Instance().firefox_path();
  QStringList args;
  args << "-v";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, true, true, 5000);
  qDebug() << "asked for firefox version"
           << "exit code:" << res.exit_code
           << "output:" << res.out;

  if (res.exit_code == 0 && res.res == SCWE_SUCCESS && !res.out.empty()) {
    for (QString str: res.out) {
      if (str.contains(QRegularExpression("Mozilla Firefox"))) {
        version = str.replace(QRegularExpression("[ a-zA-Z]"), "");
        break;
      }
    }
  }

  return SCWE_SUCCESS;
}

template<>
system_call_wrapper_error_t firefox_version_internal<Os2Type<OS_LINUX>>(QString &version) {
  return firefox_version_internal<Os2Type<OS_MAC_LIN>>(version);
}


template<>
system_call_wrapper_error_t firefox_version_internal<Os2Type<OS_MAC>>(QString &version) {
  return firefox_version_internal<Os2Type<OS_MAC_LIN>>(version);
}

template<>
system_call_wrapper_error_t firefox_version_internal<Os2Type<OS_WIN>>(QString &version) {
  version = "undefined";

  QString path;
  QString cmd("REG");
  QStringList args;
  args << "QUERY"
       << "HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\firefox.exe";
  qDebug() << "REG QUERY started"
           << "args:" << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "REG QUERY finished:"
           << "exit code:" << res.exit_code
           << "output:" << res.out;
  if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
    qCritical() << "REG QUERY failed.";
    return SCWE_CREATE_PROCESS;
  }

  for (QString str: res.out) {
    if (str.contains("Path") && str.contains("REG_SZ")) {
      std::string std_str =
          str.replace(QRegularExpression("[\t\n]+"), "").toStdString();
      int id = std_str.find(" ", std_str.find("REG_SZ"));
      path = str.right(str.size() - id).trimmed();
      break;
    }
  }

  cmd = path + "\\firefox.exe";
  args.clear();
  args << "-v";
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "asked for firefox version"
           << "exit code:" << res.exit_code
           << "output:" << res.out;

  if (res.exit_code == 0 && res.res == SCWE_SUCCESS && !res.out.empty()) {
    for (QString str: res.out) {
      if (str.contains(QRegularExpression("Mozilla Firefox"))) {
        version = str.replace(QRegularExpression("[ a-zA-Z]"), "");
        break;
      }
    }
  }

  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::firefox_version(
    QString &version) {
  return firefox_version_internal<Os2Type<CURRENT_OS>>(version);
}

////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::edge_version(QString &version) {
  version = "undefined";
#ifdef RT_OS_WINDOWS
  QString cmd("REG");
  QStringList args;
  args << "QUERY"
       << "HKCR\\AppX3xxs313wwkfjhythsb8q46xdsq8d2cvv\\Application";
  qDebug() << "querying registry for edge version"
           << "cmd:" << cmd
           << "args:" << args;
  system_call_res_t res = ssystem_th(cmd, args, true, true, 5000);
  if (res.res != SCWE_SUCCESS || res.exit_code != 0 || res.out.empty()) {
    qCritical() << "Failed to query edge registry key"
                << "exit code:" << res.exit_code
                << "output:" << res.out;
    return SCWE_CREATE_PROCESS;
  }
  for (QString str: res.out) {
    if (str.contains(QRegularExpression("Microsoft.MicrosoftEdge_[0-9.]+"))) {
      int l = str.indexOf("Microsoft.MicrosoftEdge_") + 24;
      int r = str.indexOf("_", l);
      version = str.mid(l, r - l);
      break;
    }
  }
  qDebug() << "Found Edge version:" << version;
#endif
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::safari_version(QString &version) {
  version = "undefined";
#ifdef RT_OS_DARWIN
  QString path("/Applications/Safari.app/Contents/version.plist");
  QFile plist(path);
  if (!plist.open(QIODevice::ReadOnly)) {
    qCritical() << "Can't open" << path;
    return SCWE_CREATE_PROCESS;
  }
  QTextStream stream(&plist);
  QString str = stream.readAll();
  int id = str.indexOf("CFBundleShortVersionString");
  if (id == -1) {
    qCritical() << "CFBundleShortVersionString not found in" << path;
    return SCWE_CREATE_PROCESS;
  }
  int idl = str.indexOf(QRegularExpression("[0-9.]"), id);
  int idr = str.indexOf("<", idl);
  version = str.mid(idl, idr - idl);
#endif
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::firefox_last_session() {
  qDebug() << "There is no restore session option for firefox";
  QString cmd = CSettingsManager::Instance().firefox_path();
  QStringList args;
  return QProcess::startDetached(cmd, args);
}

////////////////////////////////////////////////////////////////////////////
bool CSystemCallWrapper::chrome_last_session(){
    QString cmd = CSettingsManager::Instance().chrome_path();
    QStringList args;
    args << "--restore-last-session"
         << QString("--profile-directory=%1").arg(CSettingsManager::Instance().default_chrome_profile());
    return QProcess::startDetached(cmd, args);

}
////////////////////////////////////////////////////////////////////////////

bool CSystemCallWrapper::safari_last_session() {
  qDebug() << "No restore option for safari";
  QString cmd("open");
  QStringList args;
  args << "/Applications/Safari.app";
  return QProcess::startDetached(cmd, args);
}

////////////////////////////////////////////////////////////////////////////

const QString &CSystemCallWrapper::scwe_error_to_str(
    system_call_wrapper_error_t err) {
  static QString unknown("Unknown err");
  static QString error_str[] = {"SUCCESS",
                                "Shell error",
                                "Pipe error",
                                "set_handle_info error",
                                "create process error",
                                "p2p is not installed or hasn't execute rights",
                                "can't join to swarm",
                                "container isn't ready",
                                "cannot handshake",
                                "ssh launch failed",
                                "can't get rh ip address",
                                "can't generate ssh-key",
                                "call timeout",
                                "which call failed",
                                "process crashed",
                                "scwe_last",
                                "permission denied",
                                "wrong file name",
                                "direcory does not exist",
                                "directory already exists",
                                "command failed",
                                "wrong temp path"};
  return (err >= 0 && err < SCWE_LAST) ? error_str[err] : unknown;
}
////////////////////////////////////////////////////////////////////////////

static const QString APP_AUTOSTART_KEY = "subutai-control";

template<class OS>
bool set_application_autostart_internal(bool start);

template<>
bool set_application_autostart_internal<Os2Type<OS_LINUX> >(bool start) {
  static const QString desktop_file_content_template =
      "[Desktop Entry]\n"
      "Type=Application\n"
      "Name=SubutaiControlCenter\n"
      "Exec=%1\n"
      "Hidden=false\n"
      "NoDisplay=false\n"
      "Comment=Subutai Control Center Application\n"
      "X-GNOME-Autostart-enabled=true\n";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    qCritical(
        "Couldn't get standard locations. HOME");
    CNotificationObserver::Error(QObject::tr("Unable to get the home directory. Make sure that you "
                                             "have the required Administrative privileges."), DlgNotification::N_NO_ACTION);
    return false;
  }

  QString directory_path =
      lst_standard_locations[0] + QDir::separator() + ".config/autostart";
  QDir dir(directory_path);
  if (!dir.mkpath(directory_path)) {
    qCritical(
        "Couldn't create autostart directory");
    CNotificationObserver::Error(QObject::tr("Cannot create the autostart directory. Make sure that you "
                                             "have the required Administrative privileges."), DlgNotification::N_NO_ACTION);
    return false;
  }

  QString desktop_file_path =
      directory_path + QDir::separator() + APP_AUTOSTART_KEY + ".desktop";

  QFile desktop_file(desktop_file_path);

  if (!start) {
    if (!desktop_file.exists()) return true;  // already removed from autostart.
    if (desktop_file.remove()) return true;
    qCritical(
        "Couldn't delete file : %s",
        desktop_file.errorString().toStdString().c_str());
    CNotificationObserver::Error(QObject::tr("An error has occurred while deleting file %1. Review the error details to "
                                             "fix the issue or try to delete the file manually. Details: %2")
                                     .arg(desktop_file_path)
                                     .arg(desktop_file.errorString()), DlgNotification::N_NO_ACTION);
    return false;  // removed or not . who cares?
  }

  QString desktop_file_content, cc_path;

  system_call_wrapper_error_t scr;
  scr = CSystemCallWrapper::which("SubutaiControlCenter", cc_path);
  desktop_file_content = QString(desktop_file_content_template)
                                       .arg(cc_path);
  if (scr != SCWE_SUCCESS) {
      desktop_file_content = QString(desktop_file_content_template)
                                           .arg(QApplication::applicationFilePath());
  }

  if (!desktop_file.open(QFile::Truncate | QFile::WriteOnly)) {
    qCritical(
        "Couldn't open desktop file for write");
    CNotificationObserver::Error(
        QObject::tr("An error has occurred while creating the autostart desktop file: %1. Make sure that you have the required Administrative privileges.")
            .arg(desktop_file.errorString()), DlgNotification::N_NO_ACTION);
    return false;
  }

  bool result = true;
  do {
    QByteArray content_arr = desktop_file_content.toUtf8();
    if (desktop_file.write(content_arr) != content_arr.size()) {
      qCritical(
          "Couldn't write content to autostart desktop file : %s",
          desktop_file.errorString().toStdString().c_str());
      CNotificationObserver::Error(QObject::tr(
          "Couldn't write content to autostart desktop file"), DlgNotification::N_NO_ACTION);
      result = false;
    }
  } while (0);

  desktop_file.close();
  return result;
}
/*********************/

template<>
bool set_application_autostart_internal<Os2Type<OS_MAC> >(bool start) {
  static const QString item_location = "Library/LaunchAgents/";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    qCritical(
        "Couldn't get standard locations. HOME");
    CNotificationObserver::Error(QObject::tr("Unable to get the home directory. Make sure that you have "
                                             "the required Administrative privileges."), DlgNotification::N_NO_ACTION);
    return false;
  }

  QString item_path = lst_standard_locations[0] + QDir::separator() +
                      item_location + APP_AUTOSTART_KEY + QString(".plist");

  QString content_template =
      QString(
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
          "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
          "<plist version=\"1.0\">\n"
          "<dict>\n"
          "\t<key>Label</key>\n"
          "\t<string>%1</string>\n"
          "\t<key>ProgramArguments</key>\n"
          "\t\t<array>\n"
          "\t\t\t<string>%2</string>\n"
          "\t\t</array>\n"
          "\t<key>RunAtLoad</key>\n"
          "\t<true/>\n"
          "</dict>\n"
          "</plist>\n")
          .arg(APP_AUTOSTART_KEY)
          .arg(QApplication::applicationFilePath());

  static const QString cmd("osascript");

  QFile item_file(item_path);

  do {
    if (!start) {
      if (!item_file.exists()) break;
      if (item_file.remove()) break;
      qCritical("Couldn't remove autostart script");
      break;
    }
    if (item_file.exists()) break;
    if (!item_file.open(QFile::ReadWrite)) {
      qCritical(
          "Couldn't create subutai-tray.plist file. Error : %s",
          item_file.errorString().toStdString().c_str());
      CNotificationObserver::Error(QObject::tr(item_file.errorString().toStdString().c_str()), DlgNotification::N_NO_ACTION);
      return false;
    }

    QByteArray content = content_template.toUtf8();
    if (item_file.write(content) != content.size()) {
      qCritical(
          "Didn't write whole content to plist file");
      CNotificationObserver::Error(QObject::tr("An error has occured while writing to plist."),  DlgNotification::N_NO_ACTION);
      item_file.close();
      return false;
    }
    item_file.close();
  } while (0);

  QStringList args;
  QString add_command = QString("launchctl load %1").arg(item_path);
  QString rem_command = QString("launchctl unload %1").arg(item_path);
  args << "-e"
       << QString("do shell script \"%1\"")
              .arg(start ? add_command : rem_command);
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, false, true);
  return res.res == SCWE_SUCCESS;
}
/*********************/

template<>
bool set_application_autostart_internal<Os2Type<OS_WIN>> (bool start) {
  QStringList lst = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (lst.empty()) {
    qCritical() << "Couldn't get standard home location";
    return false;
  }

  QString vbs_path = *lst.begin() +
      "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\SubutaiControlCenterStartupScript.vbs";

  if (start) {
    if (!QFile::exists(vbs_path)) {
      QFile vbs_file(vbs_path);
      if (!vbs_file.open(QIODevice::ReadWrite)) {
        qCritical() << "Couldn't create startup script.";
        return false;
      }
      QString script = "On Error Resume Next\n"
                       "Set WshShell = CreateObject(\"WScript.Shell\")\n"
                       "WshShell.Run \"%1\"\n"
                       "Set WshShell = Nothing\n";
      vbs_file.write(script.arg(QCoreApplication::applicationFilePath())
                     .toStdString().c_str());
      vbs_file.close();
    }
  } else {
    if (QFile::exists(vbs_path)) {
      return QFile::remove(vbs_path);
    }
  }
  return true;
}
/*********************/

bool CSystemCallWrapper::set_application_autostart(bool start) {
  return set_application_autostart_internal<Os2Type<CURRENT_OS> >(start);
}
////////////////////////////////////////////////////////////////////////////

template<class OS>
bool application_autostart_internal();

template<>
bool application_autostart_internal<Os2Type<OS_LINUX> >() {
  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    qCritical(
        "Couldn't get standard locations. HOME");
    return false;
  }

  QString directory_path =
      lst_standard_locations[0] + QDir::separator() + ".config/autostart";
  QString desktop_file_path =
      directory_path + QDir::separator() + APP_AUTOSTART_KEY + ".desktop";

  QFile desktop_file(desktop_file_path);
  return desktop_file.exists();  // todo check exec field. should be
                                 // QApplication::applicationFilePath()
}
/*********************/

template<>
bool application_autostart_internal<Os2Type<OS_MAC> >() {
  static const QString item_location = "Library/LaunchAgents/";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    qCritical(
        "Couldn't get standard locations. HOME");
    return false;
  }

  QString item_path = lst_standard_locations[0] + QDir::separator() +
                      item_location + APP_AUTOSTART_KEY + QString(".plist");

  QFile item_file(item_path);
  return item_file.exists();  // todo check first argument. should be
                              // QApplication::applicationFilePath()
}
/*********************/

template<>
bool application_autostart_internal<Os2Type<OS_WIN> >() {
  QStringList lst = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (lst.empty()) {
    qCritical() << "Couldn't get standard home location";
    return false;
  }

  QString vbs_path = *lst.begin() +
      "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\SubutaiControlCenterStartupScript.vbs";
  return QFile::exists(vbs_path);
}
/*********************/

bool CSystemCallWrapper::application_autostart() {
  return application_autostart_internal<Os2Type<CURRENT_OS> >();
}

CSystemCallWrapper::container_ip_and_port
CSystemCallWrapper::container_ip_from_ifconfig_analog(const QString &port,
                                                      const QString &cont_ip,
                                                      const QString &rh_ip) {
  container_ip_and_port res;
  res.ip = QString(rh_ip);
  res.port = QString(port);
  res.use_p2p = true;
  QHostAddress template_ip(rh_ip);

  for (QHostAddress address : QNetworkInterface::allAddresses()) {
    if (address == QHostAddress::LocalHost) continue;
    if (address != template_ip) continue;  // todo use isEqual
    res.port = "22";                       // MAGIC!!
    res.ip = QString(cont_ip);
    res.use_p2p = false;
  }

  return res;
}
////////////////////////////////////////////////////////////////////////////
QStringList CSystemCallWrapper::lsb_release(){
    qDebug()
            <<"Taking info about system on Linux";

    QString cmd = "lsb_release";
    QStringList args("-a");
    system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);

    qDebug()
            <<"Result about system on Linux:"
            <<"Exit code: "<<res.exit_code
            <<"Result code: "<<res.res
            <<"Output: "<<res.out;

    QStringList output = res.out;
    if(res.exit_code != 0 || res.res != SCWE_SUCCESS)
        output.clear();
    return output;
}
////////////////////////////////////////////////////////////////////////////
template<class  OS>
system_call_wrapper_error_t tray_post_update_internal();
template<>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_LINUX> > (){
  return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_WIN> > (){
  // take version by -v
  system_call_res_t rs =
      CSystemCallWrapper::ssystem_th(QCoreApplication::applicationFilePath(),
                                     QStringList() << "-v", true, true, 3000);

  if (rs.res != SCWE_SUCCESS || rs.exit_code != 0 || rs.out.empty()) {
    qCritical() << "Failed to get TRAY VERSION";
    return SCWE_CREATE_PROCESS;
  }

  QString version = *rs.out.begin();
  version = version.left(version.indexOf("branch"));
  version.remove(QRegularExpression("[a-zA-Z ]"));

  qDebug() << "tray_post_update: version:" << version;
  if (version.isEmpty()) return SCWE_CREATE_PROCESS;
  // take product code
  QString product_code = "undefined";
  QString cmd("REG");
  QStringList args;
  args
    << "QUERY"
    << "HKLM\\SOFTWARE\\WOW6432Node\\SubutaiControlCenter"
    << "/v"
    << "ProductCode";
  qDebug()<<args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"got cc product code"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
      for (QString s : res.out){
          s = s.trimmed();
          if(s.isEmpty()) continue;
          QStringList buf = s.split(" ", QString::SkipEmptyParts);
          if(buf.size() == 3){
              product_code = buf[2];
              break;
          }
      }
  }
  if (product_code == "undefined") {
    return SCWE_CREATE_PROCESS;
  }
  // update version
  args.clear();
  args
    << "add"
    << QString("HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion"
               "\\Uninstall\\%1").arg(product_code)
    << "/t" << "REG_SZ"
    << "/v" << "DisplayVersion"
    << "/d" << version
    << "/f";
  qDebug()<<args;
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"changed version of cc"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0)
    return SCWE_CREATE_PROCESS;
  return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_MAC> > (){
  // take version by -v
  system_call_res_t rs =
      CSystemCallWrapper::ssystem_th(QCoreApplication::applicationFilePath(),
                                     QStringList() << "-v", true, true, 3000);

  if (rs.res != SCWE_SUCCESS || rs.exit_code != 0 || rs.out.empty()) {
    qCritical() << "Failed to get TRAY VERSION";
    return SCWE_CREATE_PROCESS;
  }

  QString version = *rs.out.begin();
  version = version.left(version.indexOf("branch"));
  version.remove(QRegularExpression("[a-zA-Z ]"));

  QDir dir(QApplication::applicationDirPath());
  dir.cdUp();
  QFile file(dir.absolutePath() + "/Info.plist");
  qDebug() << "got Info.plist path:" << dir.absolutePath() + "/Info.plist";
  if (!file.exists()) {
    CNotificationObserver::Error("Info.plist file is missing. make sure that "
                                 "you have installed the Control Center correctly.",
                                 DlgNotification::N_NO_ACTION);
    qCritical() << "Info.plist is missing";
    return SCWE_CREATE_PROCESS;
  }
  if (file.open(QIODevice::ReadWrite)) {
    qDebug() << "reading Info.plist file";
    QTextStream stream(&file);
    QStringList plist = stream.readAll().split("\n");
    for (auto i = plist.begin(); i != plist.end(); i++) {
      QString &str = *i;
      if (str.contains("CFBundleVersion") || str.contains("CFBundleShortVersionString")) {
        auto it = i;
        QString &strv = *(++it);
        int ida = (int) strv.toStdString().find('>');
        int idb = (int) strv.toStdString().find('<', ida);
        strv = strv.left(ida + 1) + version + strv.right(strv.size() - idb);
      }
    }
    file.close();
    if (!file.remove()) {
      CNotificationObserver::Error("Failed to remove old Info.plist file. "
                                   "Make sure that you have proper privileges.",
                                   DlgNotification::N_NO_ACTION);
      qCritical() << "Failed to remove old Info.plist file.";
      return SCWE_CREATE_PROCESS;
    }
    qDebug() << "writing into Info.plist file";
    if (!file.open(QIODevice::ReadWrite)) {
      CNotificationObserver::Error("Failed to create new Info.plist file. "
                                   "Make sure that you have proper privileges.",
                                   DlgNotification::N_NO_ACTION);
      qCritical() << "Failed to create new Info.plist file.";
      return SCWE_CREATE_PROCESS;
    }
    QTextStream sstream(&file);
    for (QString str: plist) {
      sstream << str << "\n";
    }
    file.close();
  }
  return SCWE_SUCCESS;
}
system_call_wrapper_error_t CSystemCallWrapper::tray_post_update(){
  return tray_post_update_internal<Os2Type<CURRENT_OS> >();
}
////////////////////////////////////////////////////////////////////////////
template<class OS>
system_call_wrapper_error_t p2p_post_update_internal();
template<>
system_call_wrapper_error_t p2p_post_update_internal<Os2Type<OS_LINUX> >(){
  return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t p2p_post_update_internal<Os2Type<OS_MAC> >(){
  return SCWE_SUCCESS;
}
template<>
system_call_wrapper_error_t p2p_post_update_internal<Os2Type<OS_WIN> >(){
  // get p2p product code
  // wmic product where "Name like '%Subutai p2p%'" get IdentifyingNumber
  QString cmd = "wmic";
  QStringList args;
  args
    << "product" << "where"
    << "Name like '%Subutai p2p%'"
    << "get" << "IdentifyingNumber";
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 97);
  qDebug() << "got product code of p2p"
           << "exit code: " << res.exit_code
           << "result code: " << res.res
           << "output: " << res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0) return SCWE_CREATE_PROCESS;
  QString product_code;
  if (res.out.size() == 3) {
    product_code = res.out[1];
    product_code = product_code.trimmed();
  }
  if (product_code.isEmpty()) return SCWE_CREATE_PROCESS;
  QString version;
  CSystemCallWrapper::p2p_version(version);
  version = version.remove("p2p");
  version = version.remove("version");
  version = version.trimmed();
  if (version == "undefined" || version.isEmpty()) return SCWE_SUCCESS;
  // update version
  args.clear();
  cmd = "REG";
  args
    << "add"
    << QString("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
               "\\Uninstall\\%1").arg(product_code)
    << "/t" << "REG_SZ"
    << "/v" << "DisplayVersion"
    << "/d" << version
    << "/f";
  qDebug()<<args;
  res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 3000);
  qDebug()<<"changed version of p2p"
          <<"exit code"<<res.exit_code
          <<"result code"<<res.res
          <<"output"<<res.out;
  if (res.res != SCWE_SUCCESS || res.exit_code != 0)
    return SCWE_CREATE_PROCESS;
  return SCWE_SUCCESS;
}
system_call_wrapper_error_t CSystemCallWrapper::p2p_post_update(){
  return p2p_post_update_internal<Os2Type<CURRENT_OS>>();
}
////////////////////////////////////////////////////////////////////////////
bool CSystemCallWrapper::is_host_reachable(const QString &host){
  QString cmd;
  system_call_wrapper_error_t r1 = which(QString("ping"), cmd);
  if (r1 != SCWE_SUCCESS) {
    qCritical() << "ping is not found";
    return true;
  }
  QStringList args;
#ifdef RT_OS_WINDOWS
  args << "-n" << "1" << host;
#else
  args << "-c1" << host;
#endif
  qDebug() << "ping command:" << args;
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 30000);
  qDebug() << "ping finished for " << host
           << "error code: " << res.exit_code
           << "res code: " << res.res
           << "output: " << res.out;
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res == SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////
int CProcessHandler::generate_hash() {
  while(m_proc_table[(m_hash_counter) % 1000] != nullptr) {
    m_hash_counter++; m_hash_counter %= 1000;
  }
  return m_hash_counter;
}

int CProcessHandler::sum_proc() {
  return static_cast<int>(m_proc_table.size());
}

int CProcessHandler::start_proc(QProcess &proc) {
    QMutexLocker locker(&m_proc_mutex);
    int hash = generate_hash();
    qDebug () << "Started a new proc with hash:" << hash;
    qDebug () << "Total number of procs: " << m_proc_table.size();
    m_proc_table[hash] = &proc;
    return hash;
}

void CProcessHandler::end_proc(const int &hash) {
  QMutexLocker locker(&m_proc_mutex);
  if ( m_proc_table.find(hash) != m_proc_table.end() )
    m_proc_table.erase( m_proc_table.find(hash) );
}

void CProcessHandler::clear_proc() {
  QMutexLocker locker(&m_proc_mutex);
  auto it = m_proc_table.begin();
  while (it != m_proc_table.end()) {
    if (it->second != nullptr) {
        it->second->terminate();
#ifdef RT_OS_WINDOWS
        it->second->kill();
#endif
    }
    it++;
  }
}
////////////////////////////////////////////////////////////////////////////
