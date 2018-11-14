#ifndef SSHKEYCONTROLLER_H
#define SSHKEYCONTROLLER_H
#include "qobject.h"
#include "SettingsManager.h"
#include <QStringList>
#include <QString>
#include <vector>
#include "HubController.h"

struct SshKey {
  QString file_name;
  QString path;
  QString content;
};

struct Envs {
  QString id;
  QString name;
  std::vector<SshKey> keys;
};

class SshKeyController : QObject {
Q_OBJECT
public:

  void refresh_key_files();
  void refresh_healthy_envs();
  void check_environment_keys(); // check keys exist in environment

  static SshKeyController& Instance() {
    static SshKeyController instance;
    return instance;
  }

private:
  std::vector<SshKey> m_keys;
  std::vector<CEnvironment> m_lst_healthy_environments;
  QTimer *m_timer;

  SshKeyController();
  ~SshKeyController();

private slots:
  void environments_updated_sl(int code);
  void refresh_key_files_timer_sl();

signals:
  void key_files_lst_updated();
  void progress_check_env_keys(int progress, int total);
};

#endif // SSHKEYCONTROLLER_H
