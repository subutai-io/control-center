#include <stdio.h>
#include <sstream>

#include <SystemCallWrapper.h>
#include <QApplication>
#include <QDesktopServices>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QProcess>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentRun>

#include "HubController.h"
#include "NotificationObserver.h"
#include "OsBranchConsts.h"
#include "SettingsManager.h"
#include "LibsshController.h"
#include "X2GoClient.h"

#ifdef RT_OS_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef RT_OS_WINDOWS
#include <Windows.h>
#pragma comment(lib, "Advapi32.lib")
#endif

static QString error_strings[] = {"Success",
                                  "Shell error",
                                  "Pipe error",
                                  "Set handle info error",
                                  "Create process error",
                                  "Cant join to swarm",
                                  "SSH launch error",
                                  "System call timeout"};

system_call_res_t CSystemCallWrapper::ssystem_th(const QString &cmd,
                                                 const QStringList &args,
                                                 bool read_output, bool log,
                                                 unsigned long timeout_msec) {
  QFuture<system_call_res_t> f1 =
      QtConcurrent::run(ssystem, cmd, args, read_output, log, timeout_msec);
  f1.waitForFinished();
  return f1.result();
}
////////////////////////////////////////////////////////////////////////////

system_call_res_t CSystemCallWrapper::ssystem(const QString &cmd,
                                              const QStringList &args,
                                              bool read_out, bool log,
                                              unsigned long timeout_msec) {
  QProcess proc;
  system_call_res_t res = {SCWE_SUCCESS, QStringList(), 0};

  proc.start(cmd, args);
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

system_call_wrapper_error_t CSystemCallWrapper::join_to_p2p_swarm(
    const QString &hash, const QString &key, const QString &ip, const QString &swarm_base_interface_name) {
  UNUSED_ARG(swarm_base_interface_name);

  if (is_in_swarm(hash)) return SCWE_SUCCESS;


  if (!CCommons::IsApplicationLaunchable(
          CSettingsManager::Instance().p2p_path()))
    return SCWE_P2P_IS_NOT_RUNNABLE;

  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "start"
       << "-ip" << ip
       << "-key" << key
       << "-hash" << hash
       << "-dht" << p2p_dht_arg();
  qDebug() << "ARGS=" << args;

  system_call_res_t res;
  res = ssystem_th(cmd, args, true, true, 1000 * 60 * 2); // timeout 2 min
  if (res.res != SCWE_SUCCESS)
    return res.res;

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
      ssystem_th(cmd, args, false, true);  // we don't need output. AHAHA
  return res.res;
}
////////////////////////////////////////////////////////////////////////////

template <class OS>
system_call_wrapper_error_t restart_p2p_service_internal(int *res_code);

template <>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_LINUX> >(
    int *res_code) {
  *res_code = RSE_MANUAL;

  do {
    QString gksu_path;
    system_call_wrapper_error_t scr =
        CSystemCallWrapper::which("gksu", gksu_path);
    if (scr != SCWE_SUCCESS) {
      qCritical("Couldn't find gksu command");
      break;
    }

    QString sh_path;
    scr = CSystemCallWrapper::which("sh", sh_path);
    if (scr != SCWE_SUCCESS) {
      qCritical("Couldn't find sh command");
      break;
    }

    QString systemctl_path;
    scr = CSystemCallWrapper::which("systemctl", systemctl_path);
    if (scr != SCWE_SUCCESS) {
      qCritical("Couldn't find systemctl");
      break;
    }

    QStringList args;
    args << QString("%1 list-units --all").arg(systemctl_path);
    system_call_res_t cr =
        CSystemCallWrapper::ssystem(gksu_path, args, true, true, 60000);

    if (cr.exit_code != 0 || cr.res != SCWE_SUCCESS) {
      qCritical(
          "gksu systemctl list-units call failed. ec = %d, res = %s",
          cr.exit_code,
          CSystemCallWrapper::scwe_error_to_str(cr.res).toStdString().c_str());
      break;
    }

    if (cr.out.isEmpty()) {
      qCritical(
          "gksu systemctl list-units output is empty");
      break;
    }

    for (QString str : cr.out) {
      if (str.indexOf("p2p.service") == -1) continue;

      QStringList lst_temp =
          QStandardPaths::standardLocations(QStandardPaths::TempLocation);
      if (lst_temp.empty()) {
        qCritical(
            "Couldn't get standard temp location");
        break;
      }

      QString tmpFilePath =
          lst_temp[0] + QDir::separator() + "reload_p2p_service.sh";
      qDebug() << tmpFilePath;
      QFile tmpFile(tmpFilePath);
      if (!tmpFile.open(QFile::Truncate | QFile::ReadWrite)) {
        qCritical(
            "Couldn't create reload script temp file. %s",
            tmpFile.errorString().toStdString().c_str());
        break;
      }

      QByteArray restart_script = QString(
                                      "#!/bin/bash\n"
                                      "%1 disable p2p.service\n"
                                      "%1 stop p2p.service\n"
                                      "%1 enable p2p.service\n"
                                      "%1 start p2p.service\n")
                                      .arg(systemctl_path)
                                      .toUtf8();

      if (tmpFile.write(restart_script) != restart_script.size()) {
        qCritical(
            "Couldn't write restart script to temp file");
        break;
      }
      tmpFile.close();  // save

      if (!QFile::setPermissions(
              tmpFilePath,
              QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                  QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                  QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                  QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
        qCritical(
            "Couldn't set exe permission to reload script file");
        break;
      }

      system_call_res_t cr2;
      QStringList args2;
      args2 << sh_path << tmpFilePath;
      cr2 = CSystemCallWrapper::ssystem(gksu_path, args2, false, true, 60000);
      tmpFile.remove();
      if (cr2.exit_code != 0 || cr2.res != SCWE_SUCCESS) {
        qCritical(
            "Couldn't reload p2p.service. ec = %d, err = %s", cr2.exit_code,
            CSystemCallWrapper::scwe_error_to_str(cr2.res)
                .toStdString()
                .c_str());
        break;
      }
      *res_code = RSE_SUCCESS;
      break;  // for
    }         // for

  } while (0);

  return SCWE_SUCCESS;
}
/*************************/

template <>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_WIN> >(
    int *res_code) {
  QString cmd("sc");
  QStringList args0, args1;
  args0 << "stop"
        << "\"Subutai Social P2P\"";
  args1 << "start"
        << "\"Subutai Social P2P\"";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args0, false, true);
  res = CSystemCallWrapper::ssystem_th(cmd, args1, false, true);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

template <>
system_call_wrapper_error_t restart_p2p_service_internal<Os2Type<OS_MAC> >(
    int *res_code) {
  QString cmd("osascript");
  QStringList args;
  args << "-e"
       << "do shell script \"launchctl unload "
          "/Library/LaunchDaemons/io.subutai.p2p.daemon.plist;"
          " launchctl load /Library/LaunchDaemons/io.subutai.p2p.daemon.plist\""
          " with administrator privileges";
  system_call_res_t res =
      CSystemCallWrapper::ssystem_th(cmd, args, false, true);
  *res_code = RSE_SUCCESS;
  return res.res;
}
/*************************/

system_call_wrapper_error_t CSystemCallWrapper::restart_p2p_service(
    int *res_code) {
  return restart_p2p_service_internal<Os2Type<CURRENT_OS> >(res_code);
}
////////////////////////////////////////////////////////////////////////////

system_call_wrapper_error_t CSystemCallWrapper::check_container_state(
    const QString &hash, const QString &ip) {
  QString cmd = CSettingsManager::Instance().p2p_path();
  QStringList args;
  args << "show"
       << "-hash" << hash << "-check" << ip;
  system_call_res_t res = ssystem_th(cmd, args, false, true);
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
    str_command += QString(" -i %1 ").arg(key);
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
    str_command += QString(" -i %1 ").arg(key);
  }

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
    CNotificationObserver::Instance()->Info(
        QObject::tr("Using %1 ssh key").arg(key), DlgNotification::N_NO_ACTION);
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

////////////////////////////////////////////////////////////////////////////
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

  if (res.res == SCWE_SUCCESS && res.exit_code == 0 && !res.out.empty())
    version = res.out[0];
  return res.res;
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
    version = res.out[0];
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
  version = "Couldn't get version on Win, sorry";
#if defined(RT_OS_WINDOWS)
  // todo implement  with reading registry value
#endif
  return SCWE_SUCCESS;
}

system_call_wrapper_error_t CSystemCallWrapper::chrome_version(
    QString &version) {
  return chrome_version_internal<Os2Type<CURRENT_OS> >(version);
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
                                "process crashed"};
  return (err >= 0 && err < SCWE_LAST) ? error_str[err] : unknown;
}
////////////////////////////////////////////////////////////////////////////

static const QString APP_AUTOSTART_KEY = "subutai-tray";

template <class OS>
bool set_application_autostart_internal(bool start);

template <>
bool set_application_autostart_internal<Os2Type<OS_LINUX> >(bool start) {
  static const QString desktop_file_content_template =
      "[Desktop Entry]\n"
      "Type=Application\n"
      "Name=subutai-tray\n"
      "Exec=%1\n"
      "Hidden=false\n"
      "NoDisplay=false\n"
      "Comment=subutai software\n"
      "X-GNOME-Autostart-enabled=true\n";

  QStringList lst_standard_locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

  if (lst_standard_locations.empty()) {
    qCritical(
        "Couldn't get standard locations. HOME");
    CNotificationObserver::Error(QObject::tr("Couldn't get home directory, sorry"), DlgNotification::N_NO_ACTION);
    return false;
  }

  QString directory_path =
      lst_standard_locations[0] + QDir::separator() + ".config/autostart";
  QDir dir(directory_path);
  if (!dir.mkpath(directory_path)) {
    qCritical(
        "Couldn't create autostart directory");
    CNotificationObserver::Error(QObject::tr("Couldn't create autostart directory, sorry"), DlgNotification::N_NO_ACTION);
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
    CNotificationObserver::Error(QObject::tr("Couldn't delete %1. %2")
                                     .arg(desktop_file_path)
                                     .arg(desktop_file.errorString()), DlgNotification::N_NO_ACTION);
    return false;  // removed or not . who cares?
  }

  QString desktop_file_content = QString(desktop_file_content_template)
                                     .arg(QApplication::applicationFilePath());
  if (!desktop_file.open(QFile::Truncate | QFile::WriteOnly)) {
    qCritical(
        "Couldn't open desktop file for write");
    CNotificationObserver::Error(
        QObject::tr("Couldn't create autostart desktop file. Error : %1")
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
    CNotificationObserver::Error(QObject::tr("Couldn't get home directory, sorry"), DlgNotification::N_NO_ACTION);
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
      CNotificationObserver::Error(QObject::tr("Write plist file error"),  DlgNotification::N_NO_ACTION);
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
      CNotificationObserver::Error(QObject::tr("Couldn't create registry key, sorry"),  DlgNotification::N_NO_ACTION);
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
            "Couldn't add program to autorun due to access denied. Try to run "
            "this application as administrator"), DlgNotification::N_NO_ACTION);
        result = false;
        break;
      }

      if (cr != ERROR_SUCCESS) {
        qCritical("RegSetKeyValue err : %d, %d", cr,
                                              GetLastError());
        CNotificationObserver::Error(QObject::tr("Couldn't add program to autorun, sorry"),  DlgNotification::N_NO_ACTION);
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
