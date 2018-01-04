#ifndef P2PCONTROLLER_H
#define P2PCONTROLLER_H
#include <set>

#include <QObject>
#include "SystemCallWrapper.h"
#include "HubController.h"

// Add new message to the end of array
static QString p2p_messages[] = {
  QObject::tr("The connection with environment is not established."),
  QObject::tr("Can't connect to container."),
  QObject::tr("Container doesn't have desktop."),
  QObject::tr("Press this button to ez-ssh to container."),
  QObject::tr("Press this button to ez-desktop to container.")
};

// Add new message code to the end of enum
enum p2p_message_code_t {
  CANT_JOIN_SWARM = 0,
  CANT_CONNECT_CONTAINER,
  NO_DESKTOP,
  CLICK_EZ_SSH,
  CLICK_EZ_DESKTOP
};

struct p2p_message_res_t {
  bool btn_ssh_enabled;
  bool btn_desktop_enabled;
  QString btn_ssh_message;
  QString btn_desktop_message;
};

class SwarmConnector : public QObject {
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

class SwarmLeaver : public QObject {
Q_OBJECT
public:
  SwarmLeaver();

  static SwarmLeaver& Instance() {
    static SwarmLeaver swarm_leaver;
    return swarm_leaver;
  }

  void Init() {/* need to call constructor */}
public slots:
  void leave_to_swarm_begin();
};

class HandshakeSender : public QObject {
Q_OBJECT
private:
  CEnvironment env;

public:
  HandshakeSender(const CEnvironment &_env, QObject *parent = nullptr);
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
 ~P2PController();

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
 p2p_message_res_t status(const CEnvironment *env, const CHubContainer *cont);

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
