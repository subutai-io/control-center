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

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::send_command(
    const QString &remote_user, const QString &ip, const QString &port,
    const QString &commands, const QString &key) {

  QString cmd
      = QString("%1").arg(CSettingsManager::Instance().ssh_path());
  QStringList args;
  args
       << "-o" << "StrictHostKeyChecking=no"
       << QString("%1@%2").arg(remote_user, ip)
       << "-p" << port
       << "-i" << QString("%1").arg(key)
       << QString("%1").arg(commands);
  qDebug() << "ARGS=" << args;

  system_call_res_t res = ssystem_th(cmd, args, true, true, 10000);
  qDebug() << "ARGS of command=" << args
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
      << "-P" << ssh_info.first
      << "-S" << CSettingsManager::Instance().ssh_path()
      << "-i" << ssh_info.second
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
       << "-P" << ssh_info.first
       << "-S" << CSettingsManager::Instance().ssh_path()
       << "-i" << ssh_info.second
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
        CURRENT_BRANCH == BT_MASTER ? args << "subutai/stretch-master" : args << "subutai/stretch";
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

QString CSystemCallWrapper::vagrant_fingerprint(const QString &ip){
    qDebug()
            <<"Trying to get fingerprint of "<<ip;
    QString finger = "";
    if(CRestWorker::Instance()->peer_finger(ip, finger) && !finger.isEmpty()){
        return finger.toUpper();
    }
    else{
        finger = "undefined";
        return finger;
    }
}

bool CSystemCallWrapper::vagrant_set_password(const QString &ip,
                                              const QString &username,
                                              const QString &old_pass,
                                              const QString &new_pass){
    qDebug()
            <<"Trying to set new password of "<<ip;
    return CRestWorker::Instance()->peer_set_pass(ip, username, old_pass, new_pass);
}
// parse into args line of vagrant global-status
// algo: just seperate string into 5 and parse
void parse_status_line(QString status_line,
                       QString &id,
                       QString &name,
                       QString &provider,
                       QString &state,
                       QString &directory){
    QStringList seperated = status_line.split(" ", QString::SkipEmptyParts);
    if(seperated.size() < 5) return;
    QString *args[] {&id, &name, &provider, &state};
    for (size_t i = 0; i < 4; i++){
        *args[i] = seperated[0];
        seperated.erase(seperated.begin());
    }
    directory = status_line.remove(0, status_line.indexOf(seperated[0]));
    while (!directory[directory.size() -1].isLetterOrNumber() && !directory.isEmpty()) {
        directory.remove(directory.size() -1 , 1);
    }
    return;
}

QString CSystemCallWrapper::vagrant_is_peer_update_available(const QString &ip){
    qDebug()
            <<"Trying to get update of peer: "<<ip;
    QJsonValue update_available;
    if (!CRestWorker::Instance()->peer_get_info(ip,
        "isUpdatesAvailable", update_available)) {
      return QString("false");
    }
    if(update_available.isBool()){
      return update_available.toBool() ? QString("true") : QString("false");
    }
    return QString("false");
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

QString CSystemCallWrapper::vagrant_status(const QString &dir){
    qDebug() << "get vagrant status of" << dir;
    system_call_res_t res;
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    QString status("not_created");

    args << "global-status";
    res = ssystem_th(cmd, args, true, true, 20000);
    if(res.res != SCWE_SUCCESS || res.exit_code != 0){
        return status;
    }
    QString p_name, p_id, p_state, p_provider, p_dir;
    for(auto s : res.out){
        parse_status_line(s, p_id, p_name, p_provider, p_state, p_dir);
        if(p_dir == dir){
            return p_state;
        }
    }
    return status;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_halt(const QString &dir){
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args
        << "set_working_directory"
        << dir
        << "halt";

    qDebug()
            <<"Starting to halt peer. Args:"
            <<args;
    system_call_res_t res = ssystem_th(cmd, args, true, true, 97);

    qDebug()
            <<"Halt finished:"
            <<dir
            <<res.res;
    if(res.res == SCWE_SUCCESS && res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_reload(const QString &dir){
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

    if(res.res == SCWE_SUCCESS && res.exit_code != 0){
        return SCWE_CREATE_PROCESS;
    }
    return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vagrant_destroy(const QString &dir){
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args<< "set_working_directory"
        << dir
        << "destroy"
        << "-f";

    qDebug()
            <<"Starting to destroy peer. Args:"<<args;
    system_call_res_t res = ssystem_th(cmd, args, true, true, 97);
    qDebug()<<"Destroying peer finished"
            <<"Exit code:"<<res.exit_code
            <<"Result:"<<res.res
            <<"Output:"<<res.out;
    if(res.exit_code !=0 || res.res != SCWE_SUCCESS)
        return SCWE_CREATE_PROCESS;
    QDir dir_path(dir);
    if(dir_path.removeRecursively())
        return SCWE_SUCCESS;
    else return SCWE_CREATE_PROCESS;
}

std::pair<system_call_wrapper_error_t, QStringList> CSystemCallWrapper::vagrant_up(const QString &dir){
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

QString CSystemCallWrapper::vagrant_ip(const QString &dir){
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

QString CSystemCallWrapper::vagrant_port(const QString &dir){
    QDir peer_dir(dir);
    QString  port = "undefined";
    if(peer_dir.cd(".vagrant")){
        QFile file(QString(peer_dir.absolutePath() + "/generated.yml"));
        QString file_name = file.fileName();
        if(file.exists()){
            if (file.open(QIODevice::ReadWrite) ){
                QTextStream stream( &file );
                QString output = QString(stream.readAll());
                QStringList vagrant_info = output.split("\n", QString::SkipEmptyParts);
                for (auto s : vagrant_info){
                    QString flag, value;
                    bool reading_value = false;
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
                    if(flag == "_CONSOLE_PORT")
                        port = value;
                    }
                }
            file.close();
        }
    }
    return port;
}

std::pair<QStringList, system_call_res_t> CSystemCallWrapper::vagrant_update_information(){
    qDebug() << "Starting to update information related to peer management";
    QStringList bridges = CSystemCallWrapper::list_interfaces();
    QString cmd = CSettingsManager::Instance().vagrant_path();
    QStringList args;
    args << "global-status"
         << "--prune";

    system_call_res_t global_status = ssystem_th(cmd, args, true, true, 10000);

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
QStringList CSystemCallWrapper::list_interfaces() {
  VagrantProvider::PROVIDERS provider = VagrantProvider::Instance()->CurrentProvider();
  QStringList empty;

  switch (provider) {
  case VagrantProvider::VIRTUALBOX:
    return virtualbox_interfaces();
  //case VagrantProvider::LIBVIRT:
  //  return libvirt_interfaces();
  //case VagrantProvider::HYPERV:
  //  return hyperv_interfaces();
  //case VagrantProvider::PARALLELS:
  //  return parallels_interfaces();
  default:
    return empty;
  }
}

QStringList CSystemCallWrapper::parallels_interfaces() {
  installer_is_busy.lock();

  qDebug("Getting list of bridged interfaces parallels");

  QStringList interfaces;
  QString cmd = "prlctl";
  QString empty;

  system_call_wrapper_error_t cr;

  if ((cr = CSystemCallWrapper::which(cmd, empty)) != SCWE_SUCCESS) {
    installer_is_busy.unlock();
    return interfaces;
  }

  QStringList args;
  args << "server"
       << "info"
       << "--json";

  qDebug() << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 60000);

  //QString output ="{\r\n    \"ID\": \"57bf3726-def4-43d4-a32f-cc0e8e357e16\",\r\n    \"Hostname\": \"127.0.0.1\",\r\n    \"Version\": \"Desktop 13.3.0-43321\",\r\n    \"OS\": \"Mac OS X 10.13.4(17E199)\",\r\n    \"Started as service\": \"off\",\r\n    \"VM home\": \"\\\/Users\\\/admin\\\/Parallels\",\r\n    \"Memory limit\": {\r\n        \"mode\": \"auto\"\r\n    },\r\n    \"Minimal security level\": \"low\",\r\n    \"Manage settings for new users\": \"allow\",\r\n    \"CEP mechanism\": \"off\",\r\n    \"Default encryption plugin\": \"<parallels-default-plugin>\",\r\n    \"Verbose log\": \"off\",\r\n    \"Allow mobile clients\": \"off\",\r\n    \"Proxy connection state\": \"disconnected\",\r\n    \"Direct connection\": \"off\",\r\n    \"Log rotation\": \"on\",\r\n    \"Advanced security mode\": \"off\",\r\n    \"External device auto connect\": \"ask\",\r\n    \"Proxy manager URL\": \"https:\\\/\\\/pax-manager.myparallels.com\\\/xmlrpc\\\/rpc.do\",\r\n    \"Web portal domain\": \"parallels.com\",\r\n    \"Host ID\": \"\",\r\n    \"Allow attach screenshots\": \"on\",\r\n    \"Custom password protection\": \"off\",\r\n    \"License\": {\r\n        \"state\": \"valid\",\r\n        \"key\": \"078-43914-64601-23907-20564-38467\",\r\n        \"restricted\": \"false\"\r\n    },\r\n    \"Hardware Id\": \"{d16e8ca5-3b60-5bbe-aff4-5ba1012dd0cc}\",\r\n    \"Signed In\": \"yes\",\r\n    \"Hardware info\": {\r\n        \"\/dev\/disk0\": {\r\n            \"name\": \"APPLE HDD ST1000DM003 (disk0)\",\r\n            \"type\": \"hdd\"\r\n        },\r\n        \"\/dev\/disk0s1\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"\/dev\/disk0s2\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"\/dev\/disk0s3\": {\r\n            \"name\": \"\",\r\n            \"type\": \"hdd-part\"\r\n        },\r\n        \"en0\": {\r\n            \"name\": \"Ethernet\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en1\": {\r\n            \"name\": \"Wi-Fi\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"p2p0\": {\r\n            \"name\": \"p2p0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"awdl0\": {\r\n            \"name\": \"awdl0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en2\": {\r\n            \"name\": \"en2\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"en3\": {\r\n            \"name\": \"en3\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"vnic0\": {\r\n            \"name\": \"vnic0\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"vnic1\": {\r\n            \"name\": \"vnic1\",\r\n            \"type\": \"net\"\r\n        },\r\n        \"\/dev\/cu.Bluetooth-Incoming-Port\": {\r\n            \"name\": \"\\\/dev\\\/cu.Bluetooth-Incoming-Port\",\r\n            \"type\": \"serial\"\r\n        }\r\n    }\r\n}";
  QString output;

  for (auto s : res.out) {
    output += s;
  }

  QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
  QJsonObject data = doc.object();
  QJsonObject hardware_info = data["Hardware info"].toObject();

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

  qDebug() << "Listing parallels interfaces result:"
           << "exit code: "
           << res.exit_code
           << "result: "
           << res.res
           << "output:"
           << output;

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
    return interfaces;
  }

  QStringList args; // powershell.exe -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass   -Command "$switches = Get-VMSwitch;  foreach ($switch in $switches) { Write-Debug $switch.Name }"
  args << "-NoLogo"
       << "-NoProfile"
       << "-NonInteractive"
       << "-ExecutionPolicy"
       << "Bypass"
       << "-Command"
       << "Get-VMSwitch | Select Name";

  qDebug() << cmd
           << args;

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 60000);
  QString output;

  for (auto s : res.out) {
    output += s;
  }

  qDebug() << "Listing Hyper interfaces result:"
           << "exit code: "
           << res.exit_code
           << "result: "
           << res.res
           << "output:"
           << output;

  if (res.exit_code != 0 || res.res != SCWE_SUCCESS)
      return interfaces;

  // parse virtual switches
  qDebug() << "got hyperv switches: "
           << res.out;

  QStringList parse_me = res.out;
  unsigned i = 0;
  for (auto line : parse_me) {
    if (i > 2) {
      QString bridge = line.trimmed();

      if (!bridge.isEmpty()) {
        interfaces.push_back(bridge);
        qDebug() << bridge
                 << i;
      }
    }
    i++;
  }


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
  QStringList interfaces;
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
        if(plugin.size() == 2){
            plugins.push_back(std::make_pair(plugin[0], plugin[1]));
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
                                                                const QString &box_dir){
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
  if (hash == NULL || !is_in_swarm(hash)) return SCWE_SUCCESS;
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "stop"
       << "-hash" << hash;
  system_call_res_t res =
      ssystem_th(cmd, args, false, true);  // we don't need output. AHAHA // wtf is this????
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

template <class OS>
system_call_wrapper_error_t restart_p2p_service_internal(int *res_code,restart_p2p_type type);

template <>
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
      args << "stop" << "p2p";
    } else if (type == STOPPED_P2P) {
      args << "start" << "p2p";
    } else {
      args << "restart" << "p2p";
    }
    system_call_res_t res = CSystemCallWrapper::ssystem(systemctl_path, args, true, true, 60000);
    if (res.exit_code != 0 || res.res != SCWE_SUCCESS) {
      QString err_msg = QObject::tr("Couldn't reload p2p.service. ec = %1, err = %2")
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

template <>
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

template <>
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

template <class OS>
system_call_wrapper_error_t run_sshkey_in_terminal_internal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &key);

template <>
system_call_wrapper_error_t run_sshkey_in_terminal_internal<Os2Type<OS_LINUX> >(
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

template <>
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

template <>
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
      QString("\"%1\" /k \"%2 -o UserKnownHostsFile=%3\"").arg(cmd).arg(str_command).arg(known_hosts);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(NULL, cmd_args_lpwstr, NULL, NULL, FALSE, 0, NULL,
                          NULL, &si, &pi);
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
template <class  OS>
system_call_wrapper_error_t vagrant_command_terminal_internal(const QString &dir,
                                                              const QString &command,
                                                              const QString &name);

template <>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_MAC> > (const QString &dir,
                                                                                 const QString &command,
                                                                                 const QString &name){
    if(command.isEmpty()){
        return SCWE_CREATE_PROCESS;
    }

    UNUSED_ARG(name);
    QString str_command = QString("cd %1; %2 %3 2> %4_%5;").arg(dir,
                                                                CSettingsManager::Instance().vagrant_path(),
                                                                command,
                                                                name, *(command.split(" ").begin()));
    if(command == "reload"){
        str_command += QString("%1 provision 2>> %3_%2; ").arg(CSettingsManager::Instance().vagrant_path(), command, name);
    }

    str_command += QString("echo finished > %1_finished; "
                           "echo 'Peer finished to %1 with following errors:'; "
                           "cat %2_%1; "
                           "echo 'Press any key to finish:'; "
                           "read -s -n 1; exit").arg(*(command.split(" ").begin()), name);

    QString cmd;

    cmd = CSettingsManager::Instance().terminal_cmd();
    QStringList args;
    args << QString("-e");
    qInfo("Launch command : %s",str_command.toStdString().c_str());

    args << QString("Tell application \"%1\" to %2 \"%3\"")
                .arg(cmd, CSettingsManager::Instance().terminal_arg(), str_command);
    return QProcess::startDetached(QString("osascript"), args) ? SCWE_SUCCESS
                                              : SCWE_CREATE_PROCESS;
}

template <>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_LINUX> >(const QString &dir,
                                                                                  const QString &command,
                                                                                  const QString &name){
    if(command.isEmpty()){
        return SCWE_CREATE_PROCESS;
    }

    UNUSED_ARG(name);
    QString str_command = QString("cd %1; %2 %3 2> %4_%5;").arg(dir,
                                                                CSettingsManager::Instance().vagrant_path(),
                                                                command,
                                                                name, *(command.split(" ").begin()));
    if(command == "reload"){
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
    return QProcess::startDetached(cmd, args) ? SCWE_SUCCESS
                                              : SCWE_CREATE_PROCESS;
}

template <>
system_call_wrapper_error_t vagrant_command_terminal_internal<Os2Type<OS_WIN> >(const QString &dir,
                                                                                  const QString &command,
                                                                                  const QString &name){
UNUSED_ARG(name);
UNUSED_ARG(dir);
UNUSED_ARG(command);
#ifdef RT_OS_WINDOWS

  if(command.isEmpty()){
      return SCWE_CREATE_PROCESS;
  }

  QString str_command = QString("cd %1 & %2 %3 2> %4_%5 & ").arg(dir,
                                                              CSettingsManager::Instance().vagrant_path(),
                                                              command,
                                                              name, *(command.split(" ").begin()));
  if(command == "reload"){
      str_command += QString("%1 provision 2>> %3_%2 & ").arg(CSettingsManager::Instance().vagrant_path(), command, name);
  }

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
      QString("\"%1\" /k \"%2\"").arg(cmd).arg(str_command);
  LPWSTR cmd_args_lpwstr = (LPWSTR)cmd_args.utf16();
  si.cb = sizeof(si);
  BOOL cp = CreateProcess(NULL, cmd_args_lpwstr, NULL, NULL, FALSE, 0, NULL,
                          NULL, &si, &pi);
  if (!cp) {
    qCritical(
        "Failed to create process %s. Err : %d", cmd.toStdString().c_str(),
        GetLastError());
    return SCWE_CREATE_PROCESS;
  }
#endif
  return SCWE_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::vagrant_command_terminal(const QString &dir,
                                                                         const QString &command,
                                                                         const QString &name){
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

system_call_wrapper_error_t CSystemCallWrapper::vagrant_box_remove(const QString &box,
                                                                   const QString &provider) {
  QString cmd = CSettingsManager::Instance().vagrant_path();
  QStringList args;   // vagrant box remove box_name --provider provider_name --all

  args << "box"
       << "remove"
       << box
       << "--provider"
       << provider
       << "--all";

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 10000);

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
  return res.res;
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t uninstall_p2p_internal(const QString &dir, const QString &file_name);

template <class OS>
system_call_wrapper_error_t install_p2p_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_p2p_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name){
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + "/" + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);
  if (res.exit_code != 0) {
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

template <>
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

template <>
system_call_wrapper_error_t install_p2p_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
    QString file_info = dir + "/" + file_name;
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

template <>
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

template <>
system_call_wrapper_error_t uninstall_p2p_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);

  // chrome path: /Applications/SubutaiP2P.app
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

template <>
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

system_call_wrapper_error_t CSystemCallWrapper::install_p2p(const QString &dir, const QString &file_name){
    installer_is_busy.lock();
    system_call_wrapper_error_t res = install_p2p_internal<Os2Type<CURRENT_OS> >(dir, file_name);
    installer_is_busy.unlock();
    return res;
}

system_call_wrapper_error_t CSystemCallWrapper::uninstall_p2p(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_p2p_internal<Os2Type <CURRENT_OS> >(dir, file_name);
  installer_is_busy.unlock();

  return res;
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_x2go_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + "/" + file_name;
  args << "-e"
       << QString("do shell script \"hdiutil attach %1; "
                  "cp -R /Volumes/x2goclient/x2goclient.app /Applications/x2goclient.app \" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  97);
  return res.res;
}

template <>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){
    QString cmd(dir+"/"+file_name);
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

template <>
system_call_wrapper_error_t install_x2go_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name) {
    QString file_info = dir + "/" + file_name;
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
        lst_temp[0] + QDir::separator() + "x2go_installer.sh";

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
                                    "apt-get install --yes x2goclient")
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
    cr2 = CSystemCallWrapper::ssystem(QString("pkexec"), args2, false, true, 60000);
    tmpFile.remove();

    qDebug()
            <<"installation of x2goclient finished "
           <<"error code: "<<cr2.exit_code
          <<"output: "<<cr2.out;

    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS) {
      return SCWE_CREATE_PROCESS;
    }

    return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::install_x2go(const QString &dir, const QString &file_name) {
    installer_is_busy.lock();
    system_call_wrapper_error_t res = install_x2go_internal<Os2Type <CURRENT_OS> >(dir, file_name);
    installer_is_busy.unlock();
    return res;
}

template <class OS>
system_call_wrapper_error_t uninstall_x2go_internal();

template <>
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

template <>
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
      int ind = sstd.find('"');
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

template <>
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

system_call_wrapper_error_t CSystemCallWrapper::uninstall_x2go() {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_x2go_internal<Os2Type <CURRENT_OS> >();
  installer_is_busy.unlock();

  return res;
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_vagrant_internal(const QString &dir, const QString &file_name);

template <class OS>
system_call_wrapper_error_t uninstall_vagrant_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_vagrant_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name) {
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + "/" + file_name;
  args << "-e"
       << QString("do shell script \"installer -pkg %1 -target /\" with administrator privileges").arg(file_path);
  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true,  1000 * 60 * 3);
  if (res.exit_code != 0){
    res.res = SCWE_CREATE_PROCESS;
  }
  return res.res;
}

template <>
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
template <>
system_call_wrapper_error_t install_vagrant_internal<Os2Type <OS_LINUX> >(const QString &dir, const QString &file_name){
    QString file_info = dir + "/" + file_name;
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

    cr2 = CSystemCallWrapper::ssystem(pkexec_path, args2, true, true, 60000);
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

template <>
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

  scr = CSystemCallWrapper::ssystem(QString("pkexec"), args, false, true, 60000);

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

template <>
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

template <>
system_call_wrapper_error_t uninstall_vagrant_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
  UNUSED_ARG(dir);
  UNUSED_ARG(file_name);
  // get vagrant product code
  // wmic product where "Name like '%vagrant%'" get IdentifyingNumber
  QString cmd = "wmic";
  QStringList args;
  args
    << "product" << "where"
    << "Name like '%vagrant%'"
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

system_call_wrapper_error_t CSystemCallWrapper::install_vagrant(const QString &dir, const QString &file_name){
   installer_is_busy.lock();
   system_call_wrapper_error_t res = install_vagrant_internal<Os2Type <CURRENT_OS> >(dir, file_name);
   installer_is_busy.unlock();
   return res;
}

system_call_wrapper_error_t CSystemCallWrapper::uninstall_vagrant(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_vagrant_internal<Os2Type <CURRENT_OS> >(dir, file_name);
  installer_is_busy.unlock();

  return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief CSystemCallWrapper::vagrant_plugin
/// \param name of vagrant plugin
/// \param command (we use for "vagrant plugin command")
/// accepted commands are: uninstall, install, update
/// \return
///
system_call_wrapper_error_t CSystemCallWrapper::vagrant_plugin(const QString &name,
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

  system_call_res_t res = CSystemCallWrapper::ssystem_th(cmd, args, true, true, 30000);
  qDebug() << QString("Vagrant plugin %1 %2 is finished.")
              .arg(command)
              .arg(name)
           << " Exist code: "
           << res.exit_code
           << " Result: "
           << res.res
           << " Output: "
           << res.out;

  if(res.res == SCWE_SUCCESS && res.exit_code != 0)
          res.res = SCWE_CREATE_PROCESS;

  installer_is_busy.unlock();
  return res.res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_oracle_virtualbox_internal(const QString &dir, const QString &file_name);
template <>
system_call_wrapper_error_t install_oracle_virtualbox_internal<Os2Type <OS_MAC> >(const QString &dir, const QString &file_name){
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + "/" + file_name;
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
template <>
system_call_wrapper_error_t install_oracle_virtualbox_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name){
    QString cmd(dir + "/" + file_name);
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
template <>
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
    if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS)
      return SCWE_CREATE_PROCESS;

    return SCWE_SUCCESS;
}
system_call_wrapper_error_t CSystemCallWrapper::install_oracle_virtualbox(const QString &dir, const QString &file_name){
    installer_is_busy.lock();
    system_call_wrapper_error_t res = install_oracle_virtualbox_internal<Os2Type<CURRENT_OS> > (dir, file_name);
    installer_is_busy.unlock();
    return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class  OS>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal(const QString &dir, const QString &file_name);
template<>
system_call_wrapper_error_t uninstall_oracle_virtualbox_internal<Os2Type<OS_MAC> > (const QString &dir, const QString &file_name){
  qDebug() << "uninstall virtualbox on mac";
  // run downloaded uninstallation script easy
  QString cmd("osascript");
  QStringList args;
  QString file_path  = dir + "/" + file_name;
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
system_call_wrapper_error_t CSystemCallWrapper::uninstall_oracle_virtualbox(const QString &dir, const QString &file_name){
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_oracle_virtualbox_internal<Os2Type<CURRENT_OS> > (dir, file_name);
  installer_is_busy.unlock();
  return res;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t install_chrome_internal(const QString &dir, const QString &file_name);

template <class OS>
system_call_wrapper_error_t uninstall_chrome_internal(const QString &dir, const QString &file_name);

template <>
system_call_wrapper_error_t install_chrome_internal<Os2Type <OS_MAC> > (const QString &dir, const QString &file_name) {
    qInfo() << "CC started to install google chrome";
    QString cmd("osascript");
    QStringList args;
    QString file_path  = dir + "/" + file_name;
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
    QString file_info = dir + "/" + file_name;
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
      QString err_msg = QObject::tr("Couldn't install vagrant err = %1")
                               .arg(CSystemCallWrapper::scwe_error_to_str(cr2.res));
      qCritical() << err_msg;
      return SCWE_CREATE_PROCESS;
    }

    return SCWE_SUCCESS;
}

template <>
system_call_wrapper_error_t install_chrome_internal<Os2Type <OS_WIN> >(const QString &dir, const QString &file_name) {
    QString cmd(dir+"/"+file_name);
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

template <>
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
      int ind = sstd.find('"');
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

template <>
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

template <>
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

system_call_wrapper_error_t CSystemCallWrapper::install_chrome(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = install_chrome_internal <Os2Type <CURRENT_OS> > (dir, file_name);
  installer_is_busy.unlock();

  return res;
}

system_call_wrapper_error_t CSystemCallWrapper::uninstall_chrome(const QString &dir, const QString &file_name) {
  installer_is_busy.lock();
  system_call_wrapper_error_t res = uninstall_chrome_internal <Os2Type <CURRENT_OS> > (dir, file_name);
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
system_call_wrapper_error_t CSystemCallWrapper::install_e2e(){
    QString current_browser = CSettingsManager::Instance().default_browser();
    if(current_browser == "Chrome"){
        return install_e2e_chrome();
    }
    return SCWE_SUCCESS;
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

system_call_wrapper_error_t CSystemCallWrapper::uninstall_e2e() {
  QString current_browser = CSettingsManager::Instance().default_browser();
  if (current_browser == "Chrome") {
    return uninstall_e2e_chrome();
  }
  return SCWE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
system_call_wrapper_error_t CSystemCallWrapper::install_subutai_box(const QString &dir, const QString &file_name){
    installer_is_busy.lock();
    QString subutai_box = subutai_box_name();
    QString subutai_provider = VagrantProvider::Instance()->CurrentVal();
    system_call_wrapper_error_t res = vagrant_add_box(subutai_box, subutai_provider, dir + QDir::separator() + file_name);
    if(res == SCWE_SUCCESS){
        QStringList lst_home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if(lst_home.isEmpty()){
            qCritical() << "Failed to get home directory";
            installer_is_busy.unlock();
            return SCWE_CREATE_PROCESS;
        }
        QDir boxes_path(lst_home[0] + QDir::separator() + ".vagrant.d" + QDir::separator() + "boxes");
        if(!boxes_path.exists()){
            qCritical() << "Vagrant is not installed correctly";
            installer_is_busy.unlock();
            return SCWE_CREATE_PROCESS;
        }
        QStringList parsed_box = subutai_box.split("/");
        QDir new_box_path(boxes_path.absolutePath() + QDir::separator() +
                          parsed_box[0] + "-VAGRANTSLASH-" + parsed_box[1] +
                          QDir::separator() + "0" + QDir::separator() + subutai_provider);
        if(!new_box_path.exists()){
            qCritical() << "vagrant box is not correctly installed";
            installer_is_busy.unlock();
            return SCWE_CREATE_PROCESS;
        }
        QString cloud_version = CRestWorker::Instance()->get_vagrant_box_cloud_version(subutai_box, subutai_provider);
        if(cloud_version == "undefined"){
            qCritical() << "couldn't get box cloud version";
            installer_is_busy.unlock();
            return SCWE_CREATE_PROCESS;
        }
        QDir cloud_box_path(boxes_path.absolutePath() + QDir::separator() +
                          parsed_box[0] + "-VAGRANTSLASH-" + parsed_box[1] +
                          QDir::separator() + cloud_version);
        if(!cloud_box_path.exists()){
            if(!cloud_box_path.mkdir(cloud_box_path.absolutePath())){
                qCritical() << "Failed to create directory for the new installed box";
                installer_is_busy.unlock();
                return SCWE_CREATE_PROCESS;
            }
        }
        QDir move;
        if(!move.rename(new_box_path.absolutePath(), cloud_box_path.absolutePath() + QDir::separator() + subutai_provider)){
            qCritical() << "failed to move" << new_box_path.absolutePath()
                        << "to" << cloud_box_path.absolutePath();
            res = SCWE_CREATE_PROCESS;
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
                return SCWE_CREATE_PROCESS;
            }
        }
    }
    installer_is_busy.unlock();
    return res;
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
template <class OS>
system_call_wrapper_error_t run_sshpass_in_terminal_internal(const QString &user,
                                                         const QString &ip,
                                                         const QString &port,
                                                         const QString &pass);


template <>
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
  BOOL cp = CreateProcess(NULL, cmd_args_lpwstr, NULL, NULL, FALSE, 0, NULL,
                          NULL, &si, &pi);
  if (!cp) {
    qCritical(
        "Failed to create process %s. Err : %d", cmd.toStdString().c_str(),
        GetLastError());
    return SCWE_SSH_LAUNCH_FAILED;
  }
#endif

  return SCWE_SUCCESS;
}

template <>
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

template <>
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
template <class OS>
system_call_wrapper_error_t run_x2goclient_session_internal(const QString &session_id);

template <>
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

template <>
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

template <>
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
  return res.res;
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
system_call_wrapper_error_t x2go_version_internal(QString &version);

template <>
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

template <>
system_call_wrapper_error_t x2go_version_internal <Os2Type <OS_MAC> > (QString &version){
  QString path = CSettingsManager::Instance().x2goclient();
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
        else if (line == "CFBundleVersion") {
            found = true;
        }
    }
    info_plist.close();
  }
  return SCWE_SUCCESS;
}

template <>
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
  if (x2goclient_check()) {
      version = "Installed";
      return x2go_version_internal <Os2Type <CURRENT_OS>>(version);
  } else {
    return SCWE_CREATE_PROCESS;
  }
}
////////////////////////////////////////////////////////////////////////////

template <class OS>
system_call_wrapper_error_t vagrant_version_internal(QString &version);

/********************/
template <>
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

template <>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return vagrant_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
system_call_wrapper_error_t vagrant_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return vagrant_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
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
template <class OS>
system_call_wrapper_error_t oracle_virtualbox_version_internal(QString &version);

/********************/
template <>
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

template <>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return oracle_virtualbox_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
system_call_wrapper_error_t oracle_virtualbox_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return oracle_virtualbox_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
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
system_call_wrapper_error_t vmware_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  version = "undefined";
  QStringList args;
  args << "--version";

  QString path = "/usr/bin/vmware";//CSettingsManager::Instance().oracle_virtualbox_path(); #TODO
  QDir dir(path);
  dir.cdUp();
  path = dir.absolutePath();

  system_call_res_t res = CSystemCallWrapper::ssystem_th(path,
                                                         args, true, true, 5000);

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty()) {
    version = res.out.at(0);
  }

  return res.res;
}

system_call_wrapper_error_t CSystemCallWrapper::vmware_version(QString &version) {
  return vmware_version_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////

template <class OS>
system_call_wrapper_error_t subutai_e2e_version_internal(QString &version);
template<>
system_call_wrapper_error_t subutai_e2e_version_internal<Os2Type <OS_MAC_LIN> >(QString &version){
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
system_call_wrapper_error_t subutai_e2e_version_internal<Os2Type <OS_LINUX> >(QString &version){
    return subutai_e2e_version_internal<Os2Type <OS_MAC_LIN> >(version);
}
template<>
system_call_wrapper_error_t subutai_e2e_version_internal<Os2Type <OS_MAC> >(QString &version){
    return subutai_e2e_version_internal<Os2Type <OS_MAC_LIN> >(version);
}
template<>
system_call_wrapper_error_t subutai_e2e_version_internal<Os2Type <OS_WIN> >(QString &version){
    QString current_browser = CSettingsManager::Instance().default_browser();
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (current_browser == "Chrome"){
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
    }
    if(current_browser == "Chrome" && version == "undefined"){
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
system_call_wrapper_error_t CSystemCallWrapper::subutai_e2e_version(QString &version){
    /*
     * check if chrome installed first
     */
    CSystemCallWrapper::chrome_version(version);
    if(version == "undefined"){
        version = QObject::tr("No supported browser is available");
        return SCWE_SUCCESS;
    }
    version = "undefined";
    return subutai_e2e_version_internal<Os2Type <CURRENT_OS> >(version);
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

template <class OS>
system_call_wrapper_error_t chrome_version_internal(QString &version);

/********************/
template <>
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

template <>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_LINUX> >(
    QString &version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
system_call_wrapper_error_t chrome_version_internal<Os2Type<OS_MAC> >(
    QString &version) {
  return chrome_version_internal<Os2Type<OS_MAC_LIN> >(version);
}
/********************/

template <>
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
bool CSystemCallWrapper::chrome_last_section(){
    QString cmd = CSettingsManager::Instance().chrome_path();
    QStringList args;
    args << "--restore-last-session"
         << QString("--profile-directory=%1").arg(CSettingsManager::Instance().default_chrome_profile());
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
                                "command failed"};
  return (err >= 0 && err < SCWE_LAST) ? error_str[err] : unknown;
}
////////////////////////////////////////////////////////////////////////////

static const QString APP_AUTOSTART_KEY = "subutai-control";

template <class OS>
bool set_application_autostart_internal(bool start);

template <>
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

template <>
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

template <>
bool set_application_autostart_internal<Os2Type<OS_WIN> >(bool start) {
  (void)start;  // make compiler happy
  bool result = true;
#ifdef RT_OS_WINDOWS
  HKEY rkey_run = NULL;
  static const LPCWSTR val_name((wchar_t *)APP_AUTOSTART_KEY.utf16());
  DWORD disp;
  do {  // try to write value to registry
    int32_t cr = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rkey_run, &disp);

    if (cr != ERROR_SUCCESS || !rkey_run) {
      qCritical(
          "Create registry key error. ec = %d, cr = %d", GetLastError(), cr);
      CNotificationObserver::Error(QObject::tr("An error has occurred while creating the registry key. "
                                               "Make sure that you have installed the Control Center correctly."),  DlgNotification::N_NO_ACTION);
      result = false;
      break;
    }

    if (start) {
      cr = RegSetKeyValueW(
          rkey_run, 0, val_name, REG_SZ,
          QApplication::applicationFilePath().replace("/", "\\").utf16(),
          QApplication::applicationFilePath().length() * 2);

      if (cr == ERROR_ACCESS_DENIED) {
        CNotificationObserver::Error(QObject::tr(
            "Couldn't add program to autorun due to denied access. Try to run "
            "this application as administrator"), DlgNotification::N_NO_ACTION);
        result = false;
        break;
      }

      if (cr != ERROR_SUCCESS) {
        qCritical("RegSetKeyValue err : %d, %d", cr,
                                              GetLastError());
        CNotificationObserver::Error(QObject::tr("An error has occurred while adding the program to autorun."),  DlgNotification::N_NO_ACTION);
        result = false;
        break;
      }
    } else {  // if (start)
      cr = RegDeleteKeyValueW(rkey_run, 0, val_name);

      if (cr == ERROR_ACCESS_DENIED) {
        CNotificationObserver::Error(QObject::tr(
            "Couldn't remove program from autorun due to access denied. Try to "
            "run this application as administrator"), DlgNotification::N_NO_ACTION);
        result = false;
      }

      if (cr == ERROR_PATH_NOT_FOUND) {
        result = true;
        break;
      }

      if (cr != ERROR_SUCCESS) {
        qCritical("RegDeleteKeyValueW err : %d, %d",
                                              cr, GetLastError());
        CNotificationObserver::Error(QObject::tr(
            "Couldn't remove program from autorun, sorry"), DlgNotification::N_NO_ACTION);
        result = false;
        break;
      }
    }  // if (start) ... else this block
  } while (0);

  RegCloseKey(rkey_run);
#endif
  return result;
}
/*********************/

bool CSystemCallWrapper::set_application_autostart(bool start) {
  return set_application_autostart_internal<Os2Type<CURRENT_OS> >(start);
}
////////////////////////////////////////////////////////////////////////////

template <class OS>
bool application_autostart_internal();

template <>
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

template <>
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

template <>
bool application_autostart_internal<Os2Type<OS_WIN> >() {
  bool result = true;
#ifdef RT_OS_WINDOWS
  HKEY rkey_run = NULL;
  do {
    static const LPCWSTR val_name((wchar_t *)APP_AUTOSTART_KEY.utf16());
    DWORD disp;
    int32_t cr = RegCreateKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rkey_run, &disp);

    if (cr != ERROR_SUCCESS || !rkey_run) {
      qCritical(
          "Create registry key error. ec = %d, cr = %d", GetLastError(), cr);
      result = false;
      break;
    }

    static const uint32_t buff_size = 1024;
    uint8_t buff[buff_size] = {0};
    DWORD cb_data;
    DWORD rr;
    rr = RegQueryValueEx(rkey_run, val_name, NULL, NULL, buff, &cb_data);
    if (rr != ERROR_SUCCESS) {
      result = false;
      break;
    }

    QString qdata = QString::fromUtf16((ushort *)buff, cb_data);
    result = qdata == QApplication::applicationFilePath();
  } while (0);
  RegCloseKey(rkey_run);
#endif
  return result;
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
template <class  OS>
system_call_wrapper_error_t tray_post_update_internal(const QString &version);
template <>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_LINUX> > (const QString &version){
  UNUSED_ARG(version);
  return SCWE_SUCCESS;
}
template <>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_WIN> > (const QString &version){
  UNUSED_ARG(version);
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
template <>
system_call_wrapper_error_t tray_post_update_internal<Os2Type<OS_MAC> > (const QString &version){
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
        int ida = strv.toStdString().find('>');
        int idb = strv.toStdString().find('<', ida);
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
system_call_wrapper_error_t CSystemCallWrapper::tray_post_update(const QString &version){
  return tray_post_update_internal<Os2Type<CURRENT_OS> >(version);
}
////////////////////////////////////////////////////////////////////////////
template <class OS>
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
int CProcessHandler::generate_hash(){
  while(m_proc_table[(m_hash_counter) % 1000] != nullptr) {
    m_hash_counter++; m_hash_counter %= 1000;
  }
  return m_hash_counter;
}
int CProcessHandler::sum_proc(){
    return m_proc_table.size();
}
int CProcessHandler::start_proc(QProcess &proc){
    m_proc_mutex.lock();
    int hash = generate_hash();
    qDebug () << "Started a new proc with hash:" << hash;
    qDebug () << "Total number of procs: " << m_proc_table.size();
    m_proc_table[hash] = &proc;
    m_proc_mutex.unlock();
    return hash;
}
void CProcessHandler::end_proc(const int &hash){
    m_proc_mutex.lock();
    if ( m_proc_table.find(hash) != m_proc_table.end() )
        m_proc_table.erase( m_proc_table.find(hash) );
    m_proc_mutex.unlock();
}
void CProcessHandler::clear_proc(){
    m_proc_mutex.lock();
    auto it = m_proc_table.begin();
    while (it != m_proc_table.end()){
        if (it->second != nullptr) {
            it->second->terminate();
#ifdef RT_OS_WINDOWS
            it->second->kill();
#endif
        }
        it++;
    }
    m_proc_mutex.unlock();
}
////////////////////////////////////////////////////////////////////////////

