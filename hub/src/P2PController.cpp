#include "P2PController.h"
#include <QThread>



void SwarmConnector::join_to_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::join_to_p2p_swarm(swarm_hash, swarm_key, "dhcp");
  emit join_to_swarm_finished();
}
void SwarmConnector::leave_swarm_begin() {
  system_call_wrapper_error_t res = CSystemCallWrapper::leave_p2p_swarm(swarm_hash);
  emit leave_swarm_finished();
}

void SSHtoContainer::handshake_begin() {
  system_call_wrapper_error_t err;

  CSystemCallWrapper::container_ip_and_port cip =
       CSystemCallWrapper::container_ip_from_ifconfig_analog(port, ip, rh_ip);
  if (cip.use_p2p) {
    static const int MAX_ATTEMTS_COUNT = 10;
    for (int ac = 0; ac < MAX_ATTEMTS_COUNT; ++ac) {
      err = CSystemCallWrapper::check_container_state(hash, cip.ip);
      if (err == SCWE_SUCCESS) break;
      QThread::currentThread()->sleep(1);
     }
  }
  if (err == SCWE_SUCCESS) {
    m_can_be_used = true;
  }
  else{
    m_can_be_used = false;
  }
  emit handshake_finished();
}







P2PController::P2PController()
{
  connect(&EnvironmentState::Instance(), &EnvironmentState::environemts_state_updated,
          this, &P2PController::check_environments);

}




void P2PController::join_swarm(const CEnvironment &env) {
  SwarmConnector *connector = new SwarmConnector(env.hash(), env.key());
  QThread *th = new QThread();
  connect(th, &QThread::started, connector, &SwarmConnector::join_to_swarm_begin);
  connect(connector, &SwarmConnector::join_to_swarm_finished, th, &QThread::quit);
  connector->moveToThread(th);
  th->start();
}

void P2PController::leave_swarm(const CEnvironment &env) {

  SwarmConnector *connector = new SwarmConnector(env.hash(), env.key());
  QThread *th = new QThread();
  connect(th, &QThread::started, connector, &SwarmConnector::leave_swarm_begin);
  connect(connector, &SwarmConnector::leave_swarm_finished, th, &QThread::quit);
  connector->moveToThread(th);
  th->start();

}
SSHtoContainer* P2PController::get_instance_ssh_container(const CEnvironment &env, const CHubContainer &cont) {
  std::vector<SSHtoContainer*>::iterator found_ssh = std::find_if(containers_ssh_state.begin(), containers_ssh_state.end(), [&env, &cont](const SSHtoContainer *ssh_to_container) -> bool{
    return ssh_to_container->env_id == env.id() && ssh_to_container->cont_id == cont.id();
  });
  if (found_ssh == containers_ssh_state.end())
    return NULL;
  else
    return *found_ssh;
}

void P2PController::try_to_handshake(const CEnvironment &env, const CHubContainer &cont) {

  SSHtoContainer *ssh_to_container = get_instance_ssh_container(env, cont);
  if (ssh_to_container == NULL) {
    ssh_to_container = new SSHtoContainer(cont.port(), cont.ip(), cont.rh_ip(),
                                          env.hash(), cont.id(), env.id());
    containers_ssh_state.push_back(ssh_to_container);
  }
  QThread *th = new QThread();

  connect(th, &QThread::started, ssh_to_container, &SSHtoContainer::handshake_begin);
  connect(ssh_to_container, &SSHtoContainer::handshake_finished, th, &QThread::quit);
  ssh_to_container->moveToThread(th);
  th->start();
}

bool P2PController::is_in_swarm(const CEnvironment &env) {
  return CSystemCallWrapper::is_in_swarm(env.hash());
}

void P2PController::check_environments(){
  std::vector<CEnvironment> disconnected_envs = EnvironmentState::Instance().disconnected_envs();
  std::vector<CEnvironment> current_envs = EnvironmentState::Instance().last_updated_envs();


  for (CEnvironment &env : disconnected_envs) {
    leave_swarm(env);
  }

  for (CEnvironment &env : current_envs) {
    if (is_in_swarm(env)) {
      for (CHubContainer cont : env.containers()){
        try_to_handshake(env, cont);
      }
    }
    else
      join_swarm(env);
  }
}

