#include "RestWorker.h"
#include "HubController.h"
#include "NotifiactionObserver.h"
#include "SystemCallWrapper.h"
#define UNDEFINED_BALANCE "Undefined balance"

CHubController::CHubController() :
  m_lst_environments(),
  m_lst_resource_hosts(),
  m_balance(UNDEFINED_BALANCE)
{
}

CHubController::~CHubController() {
}
////////////////////////////////////////////////////////////////////////////

int CHubController::refresh_balance() {
  int http_code, err_code;
  CSSBalance balance = CRestWorker::get_balance(http_code, err_code);
  m_balance = err_code ? QString(UNDEFINED_BALANCE) : QString("Balance: $%1").arg(balance.value());
  return 0;
}
////////////////////////////////////////////////////////////////////////////

int CHubController::refresh_environments() {
  int http_code, err_code;
  std::vector<CSSEnvironment> res = CRestWorker::get_environments(http_code, err_code);

  if (err_code) {
    CNotifiactionObserver::NotifyAboutError(QString("Refresh environments error : %1").arg(err_code));
    return 1;
  }

  if (res == m_lst_environments)
    return 1;
  m_lst_environments = res; //todo std::move
  return 0;
}
////////////////////////////////////////////////////////////////////////////

void CHubController::refresh_containers() {
  int http_code, err_code;
  std::vector<CRHInfo> res = CRestWorker::get_ssh_containers(http_code, err_code);

  if (err_code) {
    CNotifiactionObserver::NotifyAboutError(QString("Refresh containers info error : %1").arg(err_code));
    return;
  }

  if (res == m_lst_resource_hosts)
    return;
  m_lst_resource_hosts = res;
}
////////////////////////////////////////////////////////////////////////////

int CHubController::ssh_to_container(const CSSEnvironment *env,
                                      const CHubContainer *cont) {
  system_call_wrapper_error_t err = CSystemCallWrapper::join_to_p2p_swarm(env->hash().toStdString().c_str(),
                                                                          env->key().toStdString().c_str(),
                                                                          "dhcp");
  if (err != SCWE_SUCCESS) {
    CNotifiactionObserver::NotifyAboutError(QString("Failed to join to p2p network. Error : %1").arg(err));
    return SLE_JOIN_TO_SWARM_FAILED;
  }

  for (auto i = m_lst_resource_hosts.begin(); i != m_lst_resource_hosts.end(); ++i) {
    for (auto j = i->lst_containers().begin(); j != i->lst_containers().end(); ++j) {
      if (j->id() == cont->id()) {
        err = CSystemCallWrapper::run_ssh_in_terminal("subutai",
                                                      i->rh_ip().toStdString().c_str(),
                                                      cont->port().toStdString().c_str());
        if (err == SCWE_SUCCESS)
          return SLE_SUCCESS;

        CNotifiactionObserver::NotifyAboutError(QString("Run SSH failed. Error code : %1").arg((int)err));
        return SLE_SYSTEM_CALL_FAILED;
      }
    }
  }
  return SLE_CONT_NOT_FOUND;
}
////////////////////////////////////////////////////////////////////////////

int CHubController::ssh_to_container_str(const QString &env_name,
                                          const QString &cont_name) {
  CSSEnvironment *env = NULL;
  for (auto i = m_lst_environments.begin(); i != m_lst_environments.end(); ++i) {
    if (i->name() == env_name) {
      env = &(*i); break;
    }
  }
  if (env == NULL) {
    return SLE_ENV_NOT_FOUND;
  }

  const CHubContainer *cont = NULL;
  for (auto j = env->containers().begin(); j != env->containers().end(); ++j) {
    if (j->name() == cont_name) {
      cont = &(*j); break;
    }
  }

  if (cont == NULL) {
    return SLE_CONT_NOT_FOUND;
  }

  return ssh_to_container(env, cont);
}
////////////////////////////////////////////////////////////////////////////

const QString &CHubController::ssh_launch_err_to_str(int err) {
  static QString lst_err_str[] = {
    "Success", "Environment not found", "Container not found", "Join to p2p swarm failed", "System call failed" };
  return lst_err_str[err];
}
////////////////////////////////////////////////////////////////////////////
