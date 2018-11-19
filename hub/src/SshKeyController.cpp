#include "SshKeyController.h"
#include <QDir>
#include <QRegularExpression>
#include "RestWorker.h"
#include "NotificationObserver.h"
#include <QFileDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

SshKeyController::SshKeyController() {
  // clean healthy environmtns list
  m_lst_healthy_environments.clear();
  for (auto env : CHubController::Instance().lst_healthy_environments()) {
    m_lst_healthy_environments[env.id()] = env.name();
  }

  //m_timer = new QTimer;
  //m_timer->setInterval(5000);

  // refresh ssh keys list
  //refresh_key_files();

  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &SshKeyController::environments_updated_sl);
  //connect(m_timer, &QTimer::timeout,
  //        this, &SshKeyController::refresh_key_files_timer_sl);
  //m_timer->start();
  refresh_key_files_timer();
}

SshKeyController::~SshKeyController() {
  //if (m_timer)
  //  m_timer->deleteLater();
}

void SshKeyController::refresh_key_files() {
  m_mutex.lock();
  qDebug() << "SSH check timer";

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

  m_mutex.unlock();
  emit key_files_lst_updated();

  // synchronization with bazaar
  qDebug() << "CHECK ENVIRONMENT KEYS func called ";
  check_environment_keys();
  //QtConcurrent::run(this, &SshKeyController::check_environment_keys);
}

void SshKeyController::environments_updated_sl(int code) {
  UNUSED_ARG(code);
  refresh_healthy_envs();
}

void SshKeyController::refresh_healthy_envs() {
  // clean healthy environments list
  m_lst_healthy_environments.clear();
  for (auto env : CHubController::Instance().lst_healthy_environments()) {
    m_lst_healthy_environments[env.id()] = env.name();
  }

  // Todo synchronization with bazaar
  check_environment_keys();
  //QtConcurrent::run(this, &SshKeyController::check_environment_keys);
}

void SshKeyController::check_environment_keys() {
  m_mutex.lock();

  qDebug() << "CHECK ENV KEYS";

  // clean up old environment list in m_envs
  if (!m_envs.empty() && (m_envs.size() != m_lst_healthy_environments.size())) {
    std::map<QString, Envs> tmp = m_envs;

    for (auto env : tmp) {
      // not found environment(checked with bazaar) in current healthy environment list(updated)
      if (m_lst_healthy_environments.find(env.first) == m_lst_healthy_environments.end()) {
        m_envs.erase(env.first); // delete
        qDebug() << "SSH clean m_env"
                 << env.second.name
                 << env.second.id;
      }
    }
  }

  for (auto env : m_lst_healthy_environments) {
    uint8_t exist = 1;
    // found environment
    if (m_envs.find(env.first) != m_envs.end()) {
      uint8_t index = 0;
      for (auto ssh : m_keys) {
        // found ssh keys in "m_envs"
        if (std::find(m_envs[env.first].lst_ssh_contents.begin(),
                      m_envs[env.first].lst_ssh_contents.end(),
                      ssh.content) != m_envs[env.first].lst_ssh_contents.end()) {
          // check ssh key file exist in machine
          SshKey key = m_keys[index];
          QFileInfo file(key.path);

          if (!file.exists()) {
            qDebug() << "Exist ssh key in environment, "
                     << "file not found"
                     << "";
            m_envs[env.first].lst_ssh_contents.erase(
                  m_envs[env.first].lst_ssh_contents.begin() + index); // delete ssh key if not exist in machine
          } else {
            m_keys[index].env_ids << env.first;
            m_keys[index].env_ids.removeDuplicates();
          }
        }

        index = index + 1;
      }
    } else {
      QStringList tmp_ssh_key_contents;

      for (auto ssh : m_keys) {
        tmp_ssh_key_contents << ssh.content;
      }

      std::vector<uint8_t> response;
      Envs tmp_env_with_keys;
      tmp_env_with_keys.id = env.first;
      tmp_env_with_keys.name = env.second;

      QFuture<std::vector<uint8_t>> future = QtConcurrent::run(this,
                                                               &SshKeyController::check_key,
                                                               tmp_ssh_key_contents,
                                                               tmp_env_with_keys.id);
      future.waitForFinished();
      response = future.result();
      //response = CRestWorker::Instance()->is_sshkeys_in_environment(tmp_ssh_key_contents,
      //                                                              tmp_env_with_keys.id);
      uint8_t index = 0;
      for (auto i : response) {
        if (i == exist) {
          tmp_env_with_keys.keys.push_back(m_keys[index]);
          tmp_env_with_keys.lst_ssh_contents.push_back(m_keys[index].content);
          // save env id to m_keys(ssh keys list)
          m_keys[index].env_ids << tmp_env_with_keys.id;
          m_keys[index].env_ids.removeDuplicates();
        }
        index = index + 1;
      }

      m_envs[tmp_env_with_keys.id] = tmp_env_with_keys;
    }
  }

  qDebug() << "------------------------------------";
  for (auto env : m_envs) {
    qDebug() << "ENV: "
             << env.second.name;
    for (auto key : env.second.keys) {
      qDebug() << "KEYS: "
               << key.file_name;
    }
  }
  qDebug() << "------------------------------------";

  m_mutex.unlock();
  emit finished_check_environment_keys();
}

std::vector<uint8_t> SshKeyController::check_key(QStringList key_contents, QString env_id) {
  return CRestWorker::Instance()->is_sshkeys_in_environment(key_contents,
                                                            env_id);
}

void SshKeyController::refresh_key_files_timer() {
  refresh_key_files();
  QTimer::singleShot(5000, this, &SshKeyController::refresh_key_files_timer);
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
