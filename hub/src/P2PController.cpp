#include "P2PController.h"
#include <QThread>
#include <QNetworkInterface>
#include <QList>
#include <QHostAddress>
#include "Locker.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include "RestWorker.h"
#include "RhController.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

SynchroPrimitives::CriticalSection P2PConnector::m_cont_critical;
SynchroPrimitives::CriticalSection P2PConnector::m_env_critical;

void P2PConnector::join_swarm(const CEnvironment &env) {
  qInfo() <<
            QString("Trying to join the swarm for [env_name: %1, env_id: %2, swarm_hash: %3]")
             .arg(env.name())
             .arg(env.id())
             .arg(env.hash());

  SwarmConnector *swarm_connector = new SwarmConnector(env);
  connect(swarm_connector, &SwarmConnector::connection_finished, [this, env](system_call_wrapper_error_t res) {
    if (res == SCWE_SUCCESS) {
      qInfo() << QString("Joined to swarm [env_name: %1, env_id: %2, swarm_hash: %3]")
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash());
    }
    else {
      qCritical()<< QString("Can't join to swarm [env_name: %1, env_id: %2, swarm_hash: %3] Error message: %4")
                    .arg(env.name())
                    .arg(env.id())
                    .arg(env.hash())
                    .arg(CSystemCallWrapper::scwe_error_to_str(res));
    }

    SynchroPrimitives::Locker lock(&P2PConnector::m_env_critical);
    if (res == SCWE_SUCCESS)
      this->connected_envs.insert(env.hash());
    else
      this->connected_envs.erase(env.hash());
  });

  swarm_connector->startWork();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::leave_swarm(const QString &hash) {
  qInfo() << QString("This swarm is not found in hub environments. Trying to delete. [swarm_hash: %1]").arg(hash);

  SwarmLeaver *swarm_leaver = new SwarmLeaver(hash);

  connect(swarm_leaver, &SwarmLeaver::connection_finished, [this, hash](system_call_wrapper_error_t res) {
    if (res == SCWE_SUCCESS) {
      qInfo() << QString("Left the swarm [swarm_hash: %1]")
                 .arg(hash);
    }
    else {
      qCritical()<< QString("Can't leave the swarm [swarm_hash: %1] Error message: %2")
                    .arg(hash)
                    .arg(CSystemCallWrapper::scwe_error_to_str(res));
    }

    SynchroPrimitives::Locker lock(&P2PConnector::m_env_critical);
    this->connected_envs.erase(hash);
  });

  swarm_leaver->startWork();
}


//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::check_rh(const CEnvironment &env, const CHubContainer &cont) {
  qInfo() <<
            QString("Checking rh status [cont_name: %1, cont_id: %2] [env_name: %3, env_id: %4, swarm_hash: %5]")
             .arg(cont.name())
             .arg(cont.id())
             .arg(env.name())
             .arg(env.id())
             .arg(env.hash());

  RHStatusChecker *rh_checker = new RHStatusChecker(env, cont);

  connect(rh_checker, &RHStatusChecker::connection_finished, [this, env, cont](system_call_wrapper_error_t res) {
    if (res == SCWE_SUCCESS) {
      qInfo() << QString("Successfully handshaked with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash());
    }
    else {
      qInfo() << QString("Can't handshake with container [cont_name: %1, cont_id: %2] and env: [env_name: %3, env_id: %4, swarm_hash: %5]. Error message: %6")
                 .arg(cont.name())
                 .arg(cont.id())
                 .arg(env.name())
                 .arg(env.id())
                 .arg(env.hash())
                 .arg(CSystemCallWrapper::scwe_error_to_str(res));
    }
    SynchroPrimitives::Locker lock(&P2PConnector::m_cont_critical);
    if (res == SCWE_SUCCESS)
      this->connected_conts.insert(std::make_pair(env.hash(), cont.id()));
    else
      this->connected_conts.erase(std::make_pair(env.hash(), cont.id()));
  });

  rh_checker->startWork();
}


void P2PConnector::check_status(const CEnvironment &env) {
  qInfo() <<
            QString("Checking status of environment [env_name: %1, env_id: %2, swarm_hash: %3]")
             .arg(env.name())
             .arg(env.id())
             .arg(env.hash());
  for (CHubContainer cont : env.containers()) {
    check_rh(env, cont);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void P2PConnector::update_status() {
  // find if container is on your machine
  std::vector<CEnvironment> hub_environments = CHubController::Instance().lst_environments();
  std::vector<std::pair<QString, QString> > network_peers;
  QList<QHostAddress> ip_addresses = QNetworkInterface::allAddresses();
  qDebug() << "List of all lan peers:";
  for (std::pair<QString, QString> local_peer :
       CRhController::Instance()->dct_resource_hosts()) {
    network_peers.push_back(std::make_pair(
        CCommons::GetFingerprintFromUid(local_peer.first), local_peer.second));
    qDebug() << CCommons::GetFingerprintFromUid(local_peer.first) << " " << local_peer.second;
  }
  for (CEnvironment env : hub_environments) {
    for (CHubContainer cont : env.containers()) {
      bool found = false;
      QString peer_id = cont.peer_id();
      qDebug() << "Checking container's rh" << peer_id;
      for (auto lan_peer : network_peers) {
        if (lan_peer.first == peer_id) { // found in the same network
          for (auto ip : ip_addresses) {
            qDebug() << "comparing ip's of" << lan_peer.second
                     << " " << ip.toString();
            if (ip.toString() == lan_peer.second) {
              qDebug() << "Found cont in the machine"
                       << "ip: " << ip.toString();
              found = true;
              break;
            }
          }
          if (found) break;
        }
        if (found) break;
      }
      P2PController::Instance().rh_local_tbl[peer_id] = found;
    }
  }

  if (!CCommons::IsApplicationLaunchable(CSettingsManager::Instance().p2p_path())
      || !CSystemCallWrapper::p2p_daemon_check()) {
    qDebug()<<"p2p path is:"<<CSettingsManager::Instance().p2p_path();
    qCritical() << "P2P is not launchable or p2p daemon is not running.";
    connected_conts.clear();
    connected_envs.clear();
    QTimer::singleShot(30000, this, &P2PConnector::update_status); // wait more, when p2p is not operational
    return;
  }

  qInfo() << "Starting to update connection status";


  QFuture<std::vector<QString>> res_swarm = QtConcurrent::run(CSystemCallWrapper::p2p_show);
  res_swarm.waitForFinished();

  QFuture<std::vector<std::pair<QString, QString>>>  res_interfaces = QtConcurrent::run(CSystemCallWrapper::p2p_show_interfaces);
  res_interfaces.waitForFinished();

  std::vector<QString> joined_swarms = res_swarm.result(); // swarms + interfaces
  std::vector<std::pair<QString, QString>> swarm_interfaces = res_interfaces.result(); // swarms + interfaces



  QStringList already_joined;
  QStringList hub_swarms;
  QStringList lst_interfaces;

  for (auto tt : swarm_interfaces) {
    lst_interfaces << tt.first << " -- " << tt.second << "\n";
  }
  for (auto swarm : joined_swarms){
    already_joined << swarm;
  }
  for (auto env : hub_environments){
    hub_swarms << env.hash();
  }

  qDebug()
      << "Joined swarm: " << already_joined;

  qDebug()
      << "Swarms from hub: " << hub_swarms;

  qDebug()
      << "Swarm Interfaces: " << lst_interfaces;

  // need to clear it everytime
  interface_ids.clear();

  // Setting the interfaces to swarm
  // if an interface found with command `p2p show --interfaces --bind`, then we use that interface
  for (CEnvironment &env : hub_environments) {
    if (!env.healthy())
      continue;
    std::vector<std::pair<QString, QString>>::iterator found_swarm_interface =
        std::find_if(swarm_interfaces.begin(), swarm_interfaces.end(), [&env](const std::pair<QString, QString>& swarm_interface) {
        return env.hash() == swarm_interface.first;
    });

    if (found_swarm_interface != swarm_interfaces.end()
        && env.base_interface_id() == -1) {
      QString interface = found_swarm_interface->second;
      QRegExp regExp("(-?\\d+(?:[\\.,]\\d+(?:e\\d+)?)?)");
      regExp.indexIn(interface);
      int id = regExp.capturedTexts().first().toInt();
      env.set_base_interface_id(id);
      interface_ids[id] = env.hash();
    }
  }

  // if an interface was not found with command, then we will  give the unselected interfaces
  for (CEnvironment &env : hub_environments) {
    if (!env.healthy())
      continue;
    std::vector<std::pair<QString, QString>>::iterator found_swarm_interface =
        std::find_if(swarm_interfaces.begin(), swarm_interfaces.end(), [&env](const std::pair<QString, QString>& swarm_interface) {
        return env.hash() == swarm_interface.first;
    });
    if (found_swarm_interface == swarm_interfaces.end()
        && env.base_interface_id() == -1) {
      int id = get_unselected_interface_id();
      env.set_base_interface_id(id);
      interface_ids[id] = env.hash();
    }
  }

  {
    // WARNING: critical section
    SynchroPrimitives::Locker lock(&P2PConnector::m_env_critical);
    for (CEnvironment env : hub_environments) {
      if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) == joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
        connected_envs.erase(env.hash());
      }
      else {
        connected_envs.insert(env.hash());
      }
    }
  }

  // joining the swarm
  for (CEnvironment env : hub_environments) {
    if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) == joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
      if (env.healthy()) {
        join_swarm(env);
      }
    }
  }

  // checking the status of containers, handshaking
  for (CEnvironment env : hub_environments) {
    if (std::find(joined_swarms.begin(), joined_swarms.end(), env.hash()) != joined_swarms.end()) { // environment not found in joined swarm hashes, we need to try to join it
      check_status(env);
    }
  }

  // checking deleted environments
  for (QString hash : joined_swarms) {
    auto found_env = std::find_if(hub_environments.begin(), hub_environments.end(), [&hash](const CEnvironment& env) {
      return env.hash() == hash;
    });
    if (found_env == hub_environments.end()) {
      leave_swarm(hash);
    }
  }
  QTimer::singleShot(15000, this, &P2PConnector::update_status);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

P2PController::P2PController() {
   qDebug() << "P2P controller is initialized";

   connector = new P2PConnector();
   QThread* thread = new QThread(this);
   connector->moveToThread(thread);

   connect(thread, &QThread::started,
           connector, &P2PConnector::update_status);

   connect(qApp, &QCoreApplication::aboutToQuit,
           thread, &QThread::quit);

   connect(thread, &QThread::finished,
           connector, &P2PConnector::deleteLater);
   connect(thread, &QThread::finished,
           thread, &QThread::deleteLater);

   QTimer::singleShot(4000, [thread](){ // Chance that the connection with hub is established after 5 sec is high
     thread->start();
   });
}


void P2PController::update_p2p_status() {
  CRestWorker::Instance()->update_p2p_status();
}

void P2PController::p2p_status_updated_sl(std::vector<CP2PInstance> new_p2p_instances,
                                          int http_code,
                                          int err_code,
                                          int network_error){
  UNUSED_ARG(http_code);
  if (err_code || network_error) {
    qCritical(
        "Refresh p2p status failed. Err_code : %d, Net_err : %d", err_code,
        network_error);
    return;
  }

  m_p2p_instances = new_p2p_instances;
}

P2PController::P2P_CONNECTION_STATUS
P2PController::is_ready(const CEnvironment&env, const CHubContainer &cont) {
  if(!connector->env_connected(env.hash()))
    return CANT_JOIN_SWARM;
  else
  if(!connector->cont_connected(env.hash(), cont.id()))
    return CANT_CONNECT_CONT;
  else
    return CONNECTION_SUCCESS;
}

P2PController::P2P_CONNECTION_STATUS
P2PController::is_swarm_connected(const CEnvironment&env) {
  if(!connector->env_connected(env.hash()))
    return CANT_JOIN_SWARM;
  else
    return CONNECTION_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

QString P2PController::p2p_connection_status_to_str(P2P_CONNECTION_STATUS status) {
  static QString str [] = {"Successfully connected",
                           "Can't join to swarm with environment",
                           "Handshaking with container"
                          };
  return str[(size_t) status];
}

ssh_desktop_launch_error_t P2PController::is_ready_sdle(const CEnvironment& env, const CHubContainer& cont) {
  P2P_CONNECTION_STATUS ret = is_ready(env, cont);
  static ssh_desktop_launch_error_t res[] = {
    SDLE_SUCCESS,
    SDLE_JOIN_TO_SWARM_FAILED,
    SDLE_CONT_NOT_READY,
  };
  return res[(size_t) ret];
}

//p2p status updater
void P2PStatus_checker::update_status(){
    qDebug()
            <<"updating p2p status";
    if(!CCommons::IsApplicationLaunchable(CSettingsManager::Instance().p2p_path()))
        emit p2p_status(P2P_FAIL);
    else
        if(!CSystemCallWrapper::p2p_daemon_check())
           emit p2p_status(P2P_READY);
        else emit p2p_status(P2P_RUNNING);
    QTimer::singleShot(60*1000*2, this, &P2PStatus_checker::update_status); // every two minutes checks p2p
}
