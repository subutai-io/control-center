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

struct SshKey {
  QString file_name;
  QString path;
  QString content;
  QString md5;
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

static QMutex m_mutex;

class SshKeyController : public QObject {
Q_OBJECT
public:

  void refresh_key_files();
  void refresh_healthy_envs();
  void check_environment_keys(); // check keys exist in environment
  void generate_keys(QWidget* parent);

  static SshKeyController& Instance() {
    static SshKeyController instance;
    return instance;
  }

private:
  std::vector<SshKey> m_keys;
  std::map<QString, Envs> m_envs; // save exist ssh keys in environment(checked with bazaar)
  std::vector<CEnvironment> m_lst_healthy_environments;
  QTimer *m_timer;

  SshKeyController();
  ~SshKeyController();

private slots:
  void environments_updated_sl(int code);
  void refresh_key_files_timer_sl();

signals:
  void key_files_lst_updated();
  void finished_check_environment_keys();
};

#endif // SSHKEYCONTROLLER_H
