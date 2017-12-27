#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H
#include <set>

#include <QObject>
#include "SystemCallWrapper.h"
#include "HubController.h"

class SwarmConnector : public QObject{
Q_OBJECT
private:
  CEnvironment env;
  int attemptCounter;
public:
  SwarmConnector(const CEnvironment &_env, QObject *parent = nullptr);
public slots:
  void join_to_swarm_begin();
signals:
  void successfully_joined_swarm(const CEnvironment&);
  void join_swarm_timeout(const CEnvironment&);
};

class HandshakeSender : public QObject {
Q_OBJECT
private:
  CEnvironment env;

public:
  HandshakeSender(const CEnvironment &_env, QObject *parent = nullptr);
  virtual ~HandshakeSender();
  void try_to_handshake(const CHubContainer &cont);

  void handshake_begin();

signals:
  void handshake_success(QString, QString);
  void handshake_failure(QString, QString);
};

class P2PController : public QObject
{
  Q_OBJECT
private:
  QTimer* m_handshake_timer;
  QTimer* m_join_to_swarm_timer;

public:
 P2PController();

 bool join_swarm_success(QString swarm_hash);
 bool handshake_success(QString env_id, QString cont_id);
 bool has_connector (QString env_id);

 void join_swarm(const CEnvironment &env);
 void leave_swarm(const CEnvironment &env);
 void try_to_handshake(const CEnvironment &env, const CHubContainer &cont);
 void send_handshake(const CEnvironment &env, const CHubContainer &cont);
 void check_handshakes(const std::vector<CEnvironment>& envs);

 int get_container_status(const CEnvironment *env, const CHubContainer *cont);

 std::vector<CEnvironment> get_envs_without_connectors();

 std::set<QString> envs_joined_swarm_hash; // stores env_id
 std::set<QString> envs_with_connectors; // stores env_id
 std::set<std::pair<QString, QString>> successfull_handshakes; // stores env_id and cont_id

 static P2PController& Instance() {
   static P2PController instance;
   return instance;
 }

 void init(){/* need to call constructor */}

public slots:
 void handshake_with_env(const CEnvironment &env);
 void remove_connector(const CEnvironment &env);
 void joined_swarm(const CEnvironment &env);
 void handshaked(QString env_id, QString cont_id);
 void handshake_failed(QString env_id, QString cont_id);

signals:

public slots:
  void update_join_swarm_status();
  void p2p_restart();
};

#endif // P2PCONTROLLER_H
