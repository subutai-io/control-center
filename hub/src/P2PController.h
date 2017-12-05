#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H

#include <QObject>
#include "EnvironmentState.h"
#include "SystemCallWrapper.h"
class SwarmConnector : public QObject{
Q_OBJECT
private:
  QString swarm_hash, swarm_key;

public:
  SwarmConnector(QString swarm_hash, QString swarm_key)
    : swarm_hash(swarm_hash) , swarm_key(swarm_key){}

public slots:
  void join_to_swarm_begin();
  void leave_swarm_begin();

signals:
  void join_to_swarm_finished();
  void leave_swarm_finished();
};

class SSHtoContainer : public QObject
{
Q_OBJECT
public:
  QString port, ip, rh_ip, hash, cont_id, env_id;
  bool m_can_be_used;

public:
  SSHtoContainer(QString port, QString ip, QString rh_ip, QString hash, QString cont_id, QString env_id) :
    port(port), ip(ip), rh_ip(rh_ip), hash(hash), cont_id(cont_id), env_id(env_id) {}
  bool can_be_used() {return m_can_be_used;}

public slots:
   void handshake_begin();
signals:
   void handshake_finished();
};




class P2PController : public QObject
{
  Q_OBJECT
public:
 P2PController();
 void join_swarm(const CEnvironment &env);
 void leave_swarm(const CEnvironment &env);
 bool is_in_swarm(const CEnvironment &env);
 void try_to_handshake(const CEnvironment &env, const CHubContainer &cont);
 std::vector <SSHtoContainer*> containers_ssh_state;
 SSHtoContainer* get_instance_ssh_container(const CEnvironment &env, const CHubContainer &cont);

 static P2PController& Instance() {
   static P2PController instance;
   return instance;
 }

signals:

public slots:
  void check_environments();

};

#endif // P2PCONTROLLER_H
