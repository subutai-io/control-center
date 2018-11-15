#include "SshKeyController.h"
#include <QDir>
#include <QRegularExpression>
#include "RestWorker.h"
#include "NotificationObserver.h"
#include <QFileDialog>

SshKeyController::SshKeyController() {
  m_lst_healthy_environments =
      CHubController::Instance().lst_healthy_environments();

  m_timer = new QTimer;
  m_timer->setInterval(5000);

  // refresh ssh keys list
  refresh_key_files();

  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &SshKeyController::environments_updated_sl);
  connect(m_timer, &QTimer::timeout,
          this, &SshKeyController::refresh_key_files_timer_sl);
  m_timer->start();
}

SshKeyController::~SshKeyController() {
  if (m_timer)
    m_timer->deleteLater();
}

void SshKeyController::refresh_key_files() {
  m_mutex.lock();

  m_keys.clear();

  QDir dir(CSettingsManager::Instance().ssh_keys_storage());
  if (!dir.exists()) {
    qCritical(
        "Wrong ssh keys storage : %s",
        dir.absolutePath().toStdString().c_str());
    return;
  }

  QStringList name_filters({"*.pub"});
  QStringList tmp_list =
      dir.entryList(name_filters, QDir::Files | QDir::NoSymLinks);

  for (auto i = tmp_list.begin(); i != tmp_list.end(); ++i) {
    QString file_path = dir.path() + QDir::separator() + *i;
    QFile key_file(file_path);
    if (!key_file.open(QFile::ReadOnly)) {
      qCritical(
          "Can't open ssh-key file : %s, reason : %s",
          file_path.toStdString().c_str(),
          key_file.errorString().toStdString().c_str());
      continue;
    }

    QFileInfo fi(key_file);
    if (!fi.baseName().contains(QRegularExpression("[/|\\\\$%~\"*?:<>^]"))
        && !(fi.baseName()[0] == '.')) {
      SshKey key;

      QByteArray arr_content = key_file.readAll();
      arr_content.truncate(arr_content.size() - 1);  // hack for hub

      key.file_name = *i;
      key.content = QString(arr_content).remove(QRegExp("[\\n\\t\\r\\v\\f]"));
      key.path = dir.absolutePath() + QDir::separator() + (*i);
      m_keys.push_back(key);
    }
    key_file.close();
  }
  emit key_files_lst_updated();

  m_mutex.unlock();

  // todo synchronization with bazaar  
  QtConcurrent::run(this, &SshKeyController::check_environment_keys);
}

void SshKeyController::environments_updated_sl(int code) {
  if (code == CHubController::RER_NO_DIFF) return;

  refresh_healthy_envs();
}

void SshKeyController::refresh_healthy_envs() {
  m_lst_healthy_environments =
      CHubController::Instance().lst_healthy_environments();

  // Todo synchronization with bazaar
  QtConcurrent::run(this, &SshKeyController::check_environment_keys);
}

static SynchroPrimitives::CriticalSection csRbm;
void SshKeyController::check_environment_keys() {
  m_mutex.lock();

  QStringList lst_ssh_key_contents;

  for (auto key : m_keys) {
    lst_ssh_key_contents << key.content;
  }

  for (auto env : m_lst_healthy_environments) {
    uint8_t exist = 1;
    // found environment
    if (m_envs.find(env.id()) != m_envs.end()) {
      uint8_t index = 0;
      for (auto content : lst_ssh_key_contents) {
        // found ssh keys in "m_envs"
        if (std::find(m_envs[env.id()].lst_ssh_contents.begin(),
                      m_envs[env.id()].lst_ssh_contents.end(),
                      content) != m_envs[env.id()].lst_ssh_contents.end()) {
          // check ssh key exist in machine
          SshKey key = m_keys[index];
          QFileInfo file(key.path);

          if (!file.exists()) {
            m_envs[env.id()].lst_ssh_contents.erase(
                  m_envs[env.id()].lst_ssh_contents.begin() + index); // delete ssh key if not exist in machine
          }
        }

        index = index + 1;
      }
    } else {
      std::vector<uint8_t> response;
      Envs tmp_env_with_keys;
      tmp_env_with_keys.id = env.id();
      tmp_env_with_keys.name = env.name();

      response = CRestWorker::Instance()->is_sshkeys_in_environment(lst_ssh_key_contents,
                                                                    tmp_env_with_keys.id);
      {
        SynchroPrimitives::Locker lock(&csRbm);
        uint8_t index = 0;
        for (auto i : response) {
          if (i == exist) {
            tmp_env_with_keys.keys.push_back(m_keys[index]);
            tmp_env_with_keys.lst_ssh_contents.push_back(m_keys[index].content);
            index = index + 1;
          }
        }

        m_envs[tmp_env_with_keys.id] = tmp_env_with_keys;
      }
    }
  }

  // TODO clean "m_envs" map
  emit finished_check_environment_keys();
  m_mutex.unlock();
}

void SshKeyController::refresh_key_files_timer_sl() {
  refresh_key_files();
}

void SshKeyController::generate_keys(QWidget* parent) {
  QString str_file = QFileDialog::getSaveFileName(
      parent, tr("After generating the SSH key pair, you must not change the path to the SSH folder."),
      CSettingsManager::Instance().ssh_keys_storage(),
      tr("Ssh keys (*.pub);; All files (*.*)"));
  if (str_file.isEmpty()) return;
  QFileInfo fi(str_file);

  QString str_private = CSettingsManager::Instance().ssh_keys_storage() +
                        QDir::separator() + fi.baseName();

  QString str_public = str_file + (str_private == str_file ? ".pub" : "");

  QFile key(str_private);
  QFile key_pub(str_public);

  if (fi.baseName().contains(QRegularExpression("[/|\\\\$%~\"*?:<>^]")) ||
      fi.baseName().isEmpty() || fi.baseName()[0] == '.') {
    CNotificationObserver::Error(QString("SSH key name can not begin with . "
                                         "(dot) and can not contain following "
                                         "symbols:\n/|\\$%~\"<>:*?^"),
                                 DlgNotification::N_NO_ACTION);
  } else {
    if (key.exists() && key_pub.exists()) {
      key.remove();
      key_pub.remove();
    }

    system_call_wrapper_error_t scwe = CSystemCallWrapper::generate_ssh_key(
        CHubController::Instance().current_user(), str_private);
    if (scwe != SCWE_SUCCESS) {
      CNotificationObserver::Instance()->Error(
          tr("An error has occurred while trying to generate the SSH key: %1. You can manually "
             "create an SSH key or try again by restarting the Control Center first.")
              .arg(CSystemCallWrapper::scwe_error_to_str(scwe)), DlgNotification::N_NO_ACTION);
      return;
    }
  }
}
