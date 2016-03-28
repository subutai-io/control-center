#ifndef HUBCONTROLLER_H
#define HUBCONTROLLER_H

#include <vector>
#include <QString>

class CSSEnvironment;
class CRHInfo;
class CHubContainer;

typedef enum ssh_launch_error {
  SLE_SUCCESS = 0,
  SLE_ENV_NOT_FOUND,
  SLE_CONT_NOT_FOUND,
  SLE_JOIN_TO_SWARM_FAILED,
  SLE_SYSTEM_CALL_FAILED,
  SLE_LAST_ERR
} ssh_launch_error_t;

class CHubController {
private:
  std::vector<CSSEnvironment> m_lst_environments;
  std::vector<CRHInfo> m_lst_resource_hosts;
  QString m_balance;
  QString m_current_user;

  CHubController();
  ~CHubController();
  CHubController(const CHubController&);
  CHubController& operator=(const CHubController&);

public:
  int refresh_balance();
  int refresh_environments();
  void refresh_containers(); //to make ssh work

  int ssh_to_container(const CSSEnvironment *env,
                        const CHubContainer *cont);

  int ssh_to_container_str(const QString& env_id,
                            const QString& cont_id);

  static const QString& ssh_launch_err_to_str(int err);

  void set_current_user(const QString& cu) {
    m_current_user = cu;
  }

  const std::vector<CSSEnvironment>& lst_environments() const { return m_lst_environments; }
  const std::vector<CRHInfo>& lst_resource_hosts() const { return m_lst_resource_hosts; }
  const QString& balance() const {return m_balance;}
  const QString& current_user() const {return m_current_user;}

  static CHubController& Instance() {
    static CHubController instance;
    return instance;
  }
};

#endif // HUBCONTROLLER_H
