#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H
#include <set>

#include <QObject>
#include "EnvironmentState.h"
#include "SystemCallWrapper.h"

class SwarmConnector : public QObject{
Q_OBJECT
private:
  QString swarm_hash, swarm_key;
  QThread *m_th;

public:
  SwarmConnector(QString swarm_hash, QString swarm_key);
  ~SwarmConnector();

public slots:
  void join_to_swarm_begin();
signals:
  void join_to_swarm_finished();
  void successfully_joined_swarm(QString);
};

class SwarmLeaver : public QObject{
Q_OBJECT
private:
  QString swarm_hash, swarm_key;
  QThread *m_th;

public:
  SwarmLeaver(QString swarm_hash, QString swarm_key);
  ~SwarmLeaver();

public slots:
  void leave_swarm_begin();

signals:
  void leave_swarm_finished();
  void successfully_left_swarm(QString);
};




class HandshakeSender : public QObject {
Q_OBJECT
private:
  std::vector<CEnvironment> m_envs;
  QThread *m_th;

public:
  HandshakeSender(const std::vector <CEnvironment> envs);
  ~HandshakeSender();

  void try_to_handshake(const CEnvironment &env, const CHubContainer &cont);
  void send_handshakes();

signals:
  void sent_handshakes_succsessfully();
  void handshake_success(QString, QString);
  void handshake_failure(QString, QString);
};





class P2PController : public QObject
{
  Q_OBJECT
private:
  QTimer m_handshake_timer;

public:
 P2PController();

 bool join_swarm_success(QString swarm_hash);
 bool handshake_success(QString env_id, QString cont_id);

 void join_swarm(const CEnvironment &env);
 void leave_swarm(const CEnvironment &env);
 void try_to_handshake(const CEnvironment &env, const CHubContainer &cont);
 void send_handshake(const CEnvironment &env, const CHubContainer &cont);
 void check_handshakes(const std::vector<CEnvironment>& envs);

 std::vector<CEnvironment> get_joined_envs();

 std::set<QString> envs_joined_swarm_hash;
 std::set<std::pair<QString, QString>> successfull_handshakes; // stores env_id and cont_id

 static P2PController& Instance() {
   static P2PController instance;
   return instance;
 }

public slots:
 void joined_swarm(QString hash);
 void left_swarm(QString hash);
 void handshaked(QString env_id, QString cont_id);
 void handshake_failed(QString env_id, QString cont_id);

signals:

public slots:
  void update_handshake_status();
  void update_join_swarm_status();
  void update_leave_swarm_status();
};

#endif // P2PCONTROLLER_H
