#include "SshKeyController.h"
#include <QDir>
#include <QRegularExpression>
#include "RestWorker.h"
#include "NotificationObserver.h"
#include <QFileDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

Worker::Worker(const QString &file_name,
               const QString &content,
               const QStringList &env_ids) {
  m_file_name = file_name;
  m_content = content;
  m_env_ids = env_ids;
}

Worker::~Worker() {
}

void Worker::upload() { // Process. Start processing data.
  // allocate resources using new here
  qDebug("SSH upload worker running.");
  CRestWorker::Instance()->add_sshkey_to_environments(m_file_name,
                                                      m_content,
                                                      m_env_ids);
  // update environments list
  CRestWorker::Instance()->update_environments();
  emit finished();
}

void Worker::remove() {
  qDebug("SSH remove worker running.");
  CRestWorker::Instance()->remove_sshkey_from_environments(m_file_name,
                                                           m_content,
                                                           m_env_ids);
  // update environments list
  CRestWorker::Instance()->update_environments();
  emit finished();
}

SshKeyController::SshKeyController() {
  // clean healthy environments list
  m_lst_healthy_environments.clear();
  for (auto env : CHubController::Instance().lst_healthy_environments()) {
    m_lst_healthy_environments[env.id()] = env.name();
  }

  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &SshKeyController::environments_updated_sl);

  refresh_key_files_timer();
}

SshKeyController::~SshKeyController() {
  m_keys.clear();
  m_envs.clear();
}

void SshKeyController::refresh_key_files() {
  QMutexLocker locker(&m_mutex);  // Locks the mutex and unlocks when locker exits the scope
  std::vector<SshKey> tmp;
  int old_size = -1;

  if (!m_keys.empty())
    old_size = static_cast<int>(m_keys.size());

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
      tmp.push_back(key);
    }
    key_file.close();
  }

  // synchronization with bazaar (timer will run if ssh key list updated)
  if (!tmp.empty() &&  (old_size == -1 || (old_size != static_cast<int>(tmp.size())))) {
    m_keys = tmp;
    QtConcurrent::run(this, &SshKeyController::check_environment_keys);
  }
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

  QtConcurrent::run(this, &SshKeyController::check_environment_keys);
}

void SshKeyController::check_environment_keys() {
  QMutexLocker locker(&m_mutex);  // Locks the mutex and unlocks when locker exits the scope

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
    // found environment in "m_envs"
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

      uint8_t index = 0;
      std::vector<uint8_t> response = CRestWorker::Instance()->is_sshkeys_in_environment(tmp_ssh_key_contents,
                                                                                         env.first);
      Envs tmp_env_with_keys;
      tmp_env_with_keys.id = env.first;
      tmp_env_with_keys.name = env.second;

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
  for (auto key : m_keys) {
    if (!key.env_ids.empty()) {
      qDebug() << "SSH KEY HAS ENV IDS"
               << key.file_name
               << key.path
               << key.env_ids;
    }
  }
  qDebug() << "------------------------------------";

  emit finished_check_environment_keys();
}

std::vector<uint8_t> SshKeyController::check_key_rest(QStringList key_contents, QString env_id) {
  return CRestWorker::Instance()->is_sshkeys_in_environment(key_contents,
                                                            env_id);
}

void SshKeyController::refresh_key_files_timer() {
  QMutexLocker locker(&m_timer_mutex);  // Locks the mutex and unlocks when locker exits the scope
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

void SshKeyController::remove_key(const QString& file_name) {
  std::vector<SshKey> tmp = m_keys;
  for (auto key : tmp) {
    if (key.file_name == file_name) {

      // remove key in environments
      if (key.env_ids.size() > 0) {
        qDebug() << "SSH removing key from bazaar";

        emit progress_ssh_key_rest(1, 2);

        QThread* thread = new QThread;
        Worker* worker = new Worker(key.file_name,
                                    key.content,
                                    key.env_ids);
        worker->moveToThread(thread);
        connect(thread, SIGNAL (started()), worker, SLOT (remove()));
        connect(worker, &Worker::finished, [this, key] () {
          QString file_path = key.path;
          QFile key_file_pub(file_path);
          QFile key_file_private(file_path.remove(".pub"));

          // delete key
          if (key_file_pub.exists()) {
            key_file_pub.remove();
            key_file_private.remove();
          }

          emit this->progress_ssh_key_rest(2, 2);
        });
        connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
        connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
        connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
        thread->start();

      } else {
        qDebug() << "SSH removing key from machine";
        QFile key_file_pub(key.path);
        QFile key_file_private(key.path.remove(".pub"));

        if (key_file_pub.exists()) {
          key_file_pub.remove();
          key_file_private.remove();
        }
        emit progress_ssh_key_rest(100, 100);
      }
      break;
    }
  }

  emit ssh_key_send_finished();
  refresh_key_files();
}

void SshKeyController::upload_key(std::map<int, EnvsSelectState> key_with_selected_envs) {
  std::vector<SshKey> tmp = m_keys;
  qDebug() << "UPLOAD KEYS";
  int total = static_cast<int>(key_with_selected_envs.size());
  int count = 1;

  for (auto state : key_with_selected_envs) {
    SshKey key;
    size_t index = static_cast<size_t>(state.first);

    if (tmp.size() > index)
      key = tmp.at(index);

    QStringList env_ids;

    // get selected environment
    for (auto st : state.second.states) {
      if (st.second && !key.env_ids.contains(st.first))
        env_ids << st.first;
    }

    qDebug() << "UPLOAD ENV ids"
             << env_ids;

    emit progress_ssh_key_rest(count++, total);

    if (!env_ids.empty()) {

      QThread* thread = new QThread;
      Worker* worker = new Worker(key.file_name,
                                  key.content,
                                  env_ids);
      worker->moveToThread(thread);
      connect(thread, SIGNAL (started()), worker, SLOT (upload()));
      connect(worker, &Worker::finished, [env_ids, this]() {
        // clean up m_envs
        this->clean_environment_list(env_ids);
      });
      connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
      connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
      connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
      thread->start();
    }
  }

  emit ssh_key_send_finished();
  refresh_key_files();
}

void SshKeyController::clean_environment_list(const QStringList& env_ids) {
  QMutexLocker locker(&m_mutex);  // Locks the mutex and unlocks when locker exits the scope

  for (auto env_id : env_ids) {
    m_envs.erase(env_id);
  }
}
