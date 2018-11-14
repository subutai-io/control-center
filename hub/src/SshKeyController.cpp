#include "SshKeyController.h"
#include <QDir>
#include <QRegularExpression>

void SshKeyController::refresh_key_files() {
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
  // todo synchronization with bazaar
  check_environment_keys();
}

void SshKeyController::environments_updated_sl(int code) {
  if (code == CHubController::RER_NO_DIFF) return;

  refresh_healthy_envs();
}

void SshKeyController::refresh_healthy_envs() {
  m_lst_healthy_environments =
      CHubController::Instance().lst_healthy_environments();

  // Todo synchronization with bazaar
  check_environment_keys();
}

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

void SshKeyController::check_environment_keys() {

}
