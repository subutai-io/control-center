#include "EnvironmentState.h"

EnvironmentState::EnvironmentState(){
  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &EnvironmentState::on_environments_update);
}

void EnvironmentState::on_environments_update(int res) {
  UNUSED_ARG(res);

  m_connected_envs.clear();
  m_disconnected_envs.clear();

  std::vector<CEnvironment> current_envs = CHubController::Instance().lst_environments();

  // find new environments

  for(CEnvironment last_env : m_last_updated_envs) {
    std::vector<CEnvironment>::iterator
        found_cur_env = std::find_if(current_envs.begin(), current_envs.end(), [&last_env](const CEnvironment &cur_env) -> bool{
          return cur_env.id() ==  last_env.id();
        });
    if (found_cur_env == current_envs.end()) {  // then the new environmenet is added
      m_connected_envs.push_back(*found_cur_env);

    }
  }


  // find deleted environments
  for(CEnvironment cur_env : current_envs) {
    std::vector<CEnvironment>::iterator
        found_last_env = std::find_if(m_last_updated_envs.begin(), m_last_updated_envs.end(), [&cur_env](const CEnvironment &last_env) -> bool{
          return cur_env.id() ==  last_env.id();
        });
    if (found_last_env == m_last_updated_envs.end()) {  // then environment was destroyed or deleted
      m_connected_envs.push_back(*found_last_env);
    }
  }

  m_last_updated_envs.clear();
  for (CEnvironment env : current_envs) {
    m_last_updated_envs.push_back(env);
  }
  emit environemts_state_updated();
}






