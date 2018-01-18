#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H
#include <set>

#include <QObject>
#include "SystemCallWrapper.h"
#include "HubController.h"
#include <QtConcurrent/QtConcurrent>

class StatusChecker : public QObject {
  Q_OBJECT
public:
  virtual void startWork() {
    QThread* thread = new QThread;
    this->moveToThread(thread);
    connect(thread, &QThread::started,
            this, &StatusChecker::run_checker);
    connect(this, &StatusChecker::connection_finished,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &StatusChecker::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    thread->start();
  }
private slots:
  virtual void run_checker() = 0;
signals:
  virtual void connection_finished(system_call_wrapper_error_t res);

};
Q_DECLARE_INTERFACE(StatusChecker, "StatusChecker")



class RHStatusChecker : public StatusChecker
{
  Q_OBJECT

public:
  CEnvironment env;
  CHubContainer cont;
  RHStatusChecker(const CEnvironment &_env, const CHubContainer &_cont) : env(_env), cont(_cont) {}

private slots:
  void run_checker() {
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::check_container_state, env.hash(), cont.rh_ip());
    res.waitForFinished();
    emit connection_finished(res.result());
  }
};




class SwarmConnector : public StatusChecker
{
  Q_OBJECT
  public:
  CEnvironment env;
  SwarmConnector(const CEnvironment &_env) : env(_env) {}

private slots:
  void run_checker() {
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::join_to_p2p_swarm, env.hash(), env.key(), QString("dhcp"));
    res.waitForFinished();
    emit connection_finished(res.result());
  }
};



class SwarmLeaver : public StatusChecker
{
  Q_OBJECT
  public:
  QString hash;
  SwarmLeaver(const QString &_hash) : hash(_hash) {}

private slots:
  void run_checker() {
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::leave_p2p_swarm, hash);
    res.waitForFinished();
    emit connection_finished(res.result());
  }
};


class P2PConnector : public QObject {
  Q_OBJECT
public:
 bool env_connected(const QString& env_id) const {
   return connected_envs.find(env_id) != connected_envs.end();
 }
 bool cont_connected(const QString env_id, const QString& cont_id) const {
   return connected_conts.find(std::make_pair(env_id, cont_id)) != connected_conts.end();
 }

public slots:
 void update_status();

private:
 std::set< std::pair<QString, QString> > connected_conts; // Connected container. Pair of environment id and container id.
 std::set< QString > connected_envs; // Joined to swarm environment. Id of env is stored
 void join_swarm(const CEnvironment& env);
 void leave_swarm(const QString &hash);
 void check_status(const CEnvironment& env);
 void handshake(const CEnvironment& env, const CHubContainer &cont);
 void check_rh(const CEnvironment& env, const CHubContainer &cont);

};


class P2PController : public QObject
{
  Q_OBJECT

public:
  enum P2P_CONNETION_STATUS{
    CONNECTION_SUCCESS = 0,
    CANT_JOIN_SWARM,
    CANT_CONNECT_CONT,
  };

  P2PController();

  static P2PController& Instance() {
    static P2PController instance;
    return instance;
  }

  void init(){/* need to call constructor */}

  P2P_CONNETION_STATUS is_ready(const CEnvironment&env, const CHubContainer &cont);
  QString p2p_connection_status_to_str(P2P_CONNETION_STATUS status);
  ssh_desktop_launch_error_t is_ready_sdle(const CEnvironment& env, const CHubContainer& cont);

private:
  P2PConnector *connector;

};

#endif // P2PCONTROLLER_H
