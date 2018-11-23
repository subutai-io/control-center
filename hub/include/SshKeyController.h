#ifndef SSHKEYCONTROLLER_H
#define SSHKEYCONTROLLER_H
#include <QObject>
#include "SettingsManager.h"
#include <QStringList>
#include <QString>
#include <vector>
#include "HubController.h"
#include <QMutex>
#include <QMutexLocker>
#include <map>
#include "RestWorker.h"
#include "DlgGenerateSshKey.h"

struct SshKey {
  QString file_name;
  QString path;
  QString content;
  QString md5;
  QStringList env_ids; // Ssh key has many environments
};

// Saves environment ssh keys
// one to many
// Environment has many ssh keys
struct Envs {
  QString id;
  QString name;
  std::vector<QString> lst_ssh_contents;
  std::vector<SshKey> keys;
};

class SshKeyController : public QObject {
Q_OBJECT
public:

  std::vector<uint8_t> check_key_rest(QStringList key_contents,
                                      QString env_id);
  void refresh_key_files();
  void refresh_healthy_envs();
  void check_environment_keys(); // check keys exist in environment
  void generate_keys(QWidget* parent);
  std::vector<SshKey> list_keys() {
    return m_keys;
  }
  std::map<QString, QString> list_healthy_envs() {
    return m_lst_healthy_environments;
  }

  /* every 5 secs, update ssh key list
   * updates variable "m_envs"
  */
  void refresh_key_files_timer();

  bool key_exist_in_env(size_t index, QString env_id) {
    return m_keys[index].env_ids.contains(env_id);
  }

  std::vector<SshKey> keys_in_environment(const QString& env_id) {
    std::vector<SshKey> empty;

    if (m_envs.find(env_id) != m_envs.end())
      empty = m_envs[env_id].keys;

    return empty;
  }

  void remove_key(const QString& file_name);
  // key: index of ssh key (m_keys)
  // value: struct with EnvsSelectState
  void upload_key(std::map<int, EnvsSelectState> key_with_selected_envs);

  static SshKeyController& Instance() {
    static SshKeyController instance;
    return instance;
  }

private:
  // ssh keys list
  std::vector<SshKey> m_keys;

  // key: environment id, value: Struct Envs.
  // saves checked ssh keys of environment(checked with bazaar)
  std::map<QString, Envs> m_envs;

  // key: env id, value: env name.
  std::map<QString, QString> m_lst_healthy_environments;
  QMutex m_mutex; // mutex for check ssh keys with bazaar
  QMutex m_timer_mutex;

  SshKeyController();
  ~SshKeyController();

private slots:
  void environments_updated_sl(int code);

signals:
  void ssh_key_send_finished();
  void finished_check_environment_keys();
  void progress_ssh_key_rest(int current, int total);
};

#endif // SSHKEYCONTROLLER_H
