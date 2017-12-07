#ifndef ENVIRONMENTSTATE_H
#define ENVIRONMENTSTATE_H

#include <QObject>
#include "RestContainers.h"
#include "HubController.h"

class EnvironmentState : public QObject
{
  Q_OBJECT
private:
  std::vector<CEnvironment> m_last_updated_envs;
  std::vector<CEnvironment> m_connected_envs;
  std::vector<CEnvironment> m_disconnected_envs;

public:

  std::vector<CEnvironment>& last_updated_envs() {return m_last_updated_envs;}
  std::vector<CEnvironment>& connected_envs() {return m_connected_envs;}
  std::vector<CEnvironment>& disconnected_envs() {return m_disconnected_envs;}

  EnvironmentState();

  static EnvironmentState& Instance() {
    static EnvironmentState instance;
    return instance;
  }
  void init(){/*need to call constructor */}

signals:
  void environemts_state_updated();

public slots:
   void on_environments_update(int res);

};

#endif // ENVIRONMENTSTATE_H
