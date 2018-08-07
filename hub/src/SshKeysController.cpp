#include "SshKeysController.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <algorithm>

#include "Locker.h"
#include "NotificationObserver.h"
#include "RestWorker.h"
#include "SystemCallWrapper.h"

CSshKeysController::CSshKeysController()
    : m_current_key_col(-1),
      m_rows(0),
      m_cols(0),
      m_refresh_files_timer(nullptr) {
  m_lst_healthy_environments =
      CHubController::Instance().lst_healthy_environments();
  m_refresh_files_timer = new QTimer;
  m_refresh_files_timer->setInterval(5000);

  refresh_key_files();

  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &CSshKeysController::environments_updated);
  connect(m_refresh_files_timer, &QTimer::timeout, this,
          &CSshKeysController::refresh_files_timer_timeout);
  m_refresh_files_timer->start();
}

CSshKeysController::~CSshKeysController() {
  if (m_refresh_files_timer) m_refresh_files_timer->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::refresh_key_files() {
  QStringList lst_key_content;
  QStringList lst_key_files;

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
      lst_key_files.push_back(*i);
      QByteArray arr_content = key_file.readAll();
      arr_content.truncate(arr_content.size() - 1);  // hack for hub
      lst_key_content.push_back(QString(arr_content).remove(QRegExp("[\\n\\t\\r\\v\\f]")));
    }
    key_file.close();
  }

  if (lst_key_files == m_lst_key_files) return;

  m_lst_key_content.clear();
  m_lst_key_files.clear();

  m_current_key = QString();
  m_current_key_col = -1;

  m_lst_key_files = std::move(lst_key_files);
  m_lst_key_content = std::move(lst_key_content);

  if (!m_lst_key_files.empty() && m_current_key.isEmpty())
    set_current_key(m_lst_key_files[0]);

  emit key_files_changed();
  QtConcurrent::run(this, &CSshKeysController::rebuild_bit_matrix);
}
////////////////////////////////////////////////////////////////////////////

static SynchroPrimitives::CriticalSection csRbm;
void CSshKeysController::rebuild_bit_matrix() {
  qDebug("Starting to rebild ssh keys bitMatrix");
  key_environment_matrix tmpMatrix;
  uint32_t rows, cols;

  QStringList lst_key_content = m_lst_key_content;
  std::vector<CEnvironment> lst_he = m_lst_healthy_environments;
  rows = (uint32_t)lst_he.size();
  cols = lst_key_content.size();

  tmpMatrix.reserve(rows);
  tmpMatrix.resize(rows);
  for (size_t row = 0; row < rows; ++row) {
    tmpMatrix[row] = std::vector<uint8_t>(cols);
  }

  for (size_t row = 0; row < rows; ++row) {
    tmpMatrix[row] = CRestWorker::Instance()->is_sshkeys_in_environment(
        lst_key_content, lst_he[row].id());
  }

  {
    SynchroPrimitives::Locker lock(&csRbm);
    m_current_ke_matrix = std::move(tmpMatrix);
    m_original_ke_matrix.erase(m_original_ke_matrix.begin(),
                               m_original_ke_matrix.end());
    std::copy(m_current_ke_matrix.begin(), m_current_ke_matrix.end(),
              std::back_inserter(m_original_ke_matrix));
    m_rows = rows;
    m_cols = cols;
    m_lst_all_selected.clear();
    m_lst_all_selected.reserve(m_cols);
    m_lst_all_selected.resize(m_cols);
  }
  qDebug("Finished rebuilding ssh keys bitMatrix");
  emit matrix_updated();
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::reset_matrix_current() {
    rebuild_bit_matrix();
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::generate_new_ssh_key(QWidget *parent) {
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

////////////////////////////////////////////////////////////////////////////

void CSshKeysController::remove_ssh_key() {
  if (m_current_key.isEmpty()) {
    return;
  }
  QString text, list_deployed;

  for (int i = 0; i < (int) m_lst_healthy_environments.size(); i++) {
    if (m_original_ke_matrix.size() < i ||
        m_current_key_col < 0 ||
        m_original_ke_matrix[i].size() < m_current_key_col) {
      continue;
    }
    if (m_original_ke_matrix[i][m_current_key_col]) {
      list_deployed += "<i>" + m_lst_healthy_environments[i].name() + "</i><br>";
    }
  }

  if (!list_deployed.isEmpty()) {
    text = QString("You are going to remove an SSH-key <i>%1</i>. This SSH-key "
                   "is deployed to this environments:<br>%2Your SSH-key will be "
                   "removed from all of the environments.<br>Do you want to "
                   "proceed?").arg(m_current_key, list_deployed);
  } else {
    text = QString("You are going to remove an SSH-key <i>%1</i>. This "
                   "SSH-key is not deployed to the environments.<br> "
                   "Do you want to proceed?").arg(m_current_key);
  }

  QMessageBox *msg = new QMessageBox(
        QMessageBox::Information, "Attention!", text,
        QMessageBox::Yes | QMessageBox::No);
  msg->setTextFormat(Qt::RichText);
  connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);

  if (msg->exec() != QMessageBox::Yes) {
    return;
  }

  for (int i = 0; i < (int) m_lst_healthy_environments.size(); i++) {
    set_key_environments_bit(i, false);
  }
  send_data_to_hub();

  system_call_wrapper_error_t res =
      CSystemCallWrapper::remove_ssh_key(m_current_key);

  if (res == SCWE_WRONG_FILE_NAME) {
    CNotificationObserver::Error(
          tr("Invalid SSH-keys storage path. Please change SSH-keys "
             "storage in settings."), DlgNotification::N_SETTINGS);
    return;
  } else if (res == SCWE_PERMISSION_DENIED) {
    CNotificationObserver::Error(
          tr("You don't have administrator privileges to write into "
             "SSH directory."), DlgNotification::N_NO_ACTION);
    return;
  } else if (res != SCWE_SUCCESS) {
    CNotificationObserver::Error(
          tr("An error occured while removing selected SSH-key. "
             "Make sure that you have proper administrator privileges."),
          DlgNotification::N_NO_ACTION);
    return;
  }

  qDebug() << "SSH-key" << m_current_key << "has been removed.";
  m_current_key.clear();
}

////////////////////////////////////////////////////////////////////////////

void CSshKeysController::send_data_to_hub() {

  std::map<QString, std::pair<QString, std::vector<QString> > > dct_to_send;
  std::map<QString, std::pair<QString, std::vector<QString> > > dct_to_remove;
  std::vector<size_t> lst_to_remove_idxs;
  std::vector<CEnvironment> lst_to_leave;

  {
    SynchroPrimitives::Locker lock(&csRbm);
    std::vector<CEnvironment> lst_he = m_lst_healthy_environments;

    for (int32_t col = 0; col < m_cols; ++col) {
      QString key = m_lst_key_content[col];
      QString key_name = m_lst_key_files[col];

      for (int32_t row = 0; row < m_rows; ++row) {
        if ((int32_t)m_current_ke_matrix[row].size() <= col) {
          continue;
        }

        if (m_current_ke_matrix[row][col] == m_original_ke_matrix[row][col]) {
          continue;
        }

        if (std::find(lst_to_remove_idxs.begin(), lst_to_remove_idxs.end(),
                      row) == lst_to_remove_idxs.end()) {
          lst_to_remove_idxs.push_back(row);
        }

        if (!m_current_ke_matrix[row][col]) {
          qInfo("ssh-key %s remove from env %s",
                                                key_name.toStdString().c_str(),
                                                lst_he[row].name().toStdString().c_str());
          dct_to_remove[key].first = key_name;
          dct_to_remove[key].second.push_back(lst_he[row].id());
        } else {
          qInfo("ssh-key %s add to env %s",
                                                key_name.toStdString().c_str(),
                                                lst_he[row].name().toStdString().c_str());
          dct_to_send[key].first = key_name;
          dct_to_send[key].second.push_back(lst_he[row].id());
        }
      }  // for row
    }    // for col

    for (size_t i = 0; i < lst_he.size(); ++i) {
      if (std::find(lst_to_remove_idxs.begin(), lst_to_remove_idxs.end(), i) !=
          lst_to_remove_idxs.end())
        continue;
      lst_to_leave.push_back(lst_he[i]);
    }

    m_lst_healthy_environments = lst_to_leave;
  }
  int part = 0;
  int total = (int)(dct_to_send.size() + dct_to_remove.size());
  for (auto i = dct_to_send.begin(); i != dct_to_send.end(); ++i) {
    CRestWorker::Instance()->add_sshkey_to_environments(
        i->second.first, i->first, i->second.second);
    emit ssh_key_send_progress(++part, total);
  }

  for (auto i = dct_to_remove.begin(); i != dct_to_remove.end(); ++i) {
    CRestWorker::Instance()->remove_sshkey_from_environments(
        i->second.first, i->first, i->second.second);
    emit ssh_key_send_progress(++part, total);
  }

  emit ssh_key_send_finished();
  QFuture<void> result =
      QtConcurrent::run(this, &CSshKeysController::rebuild_bit_matrix);
  result.waitForFinished();
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::set_current_key(const QString &key) {
  m_current_key_col = m_lst_key_files.indexOf(key);
  if (m_current_key_col != -1) m_current_key = key;
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::set_key_environments_bit(int index, bool bit) {
  if (m_current_key.isEmpty()) return;
  if (index < 0 || index >= (int)m_current_ke_matrix.size()) return;
  if (m_current_key_col >= (int)m_current_ke_matrix[index].size()) return;

  m_current_ke_matrix[index][m_current_key_col] = bit;
  bit = true;
  for (int32_t row = 0; row < m_rows; ++row) {
    if (m_current_ke_matrix[row][m_current_key_col]) continue;
    bit = false;
    break;
  }
  m_lst_all_selected[m_current_key_col] = bit;
}
////////////////////////////////////////////////////////////////////////////

bool CSshKeysController::get_key_environments_bit(int index) const {
  if (m_current_key.isEmpty()) return false;
  if (index < 0 || index >= (int)m_rows) return false;
  if (m_current_ke_matrix[index].empty() || m_current_key_col >= m_cols)
    return false;
  return m_current_ke_matrix[index][m_current_key_col];
}
////////////////////////////////////////////////////////////////////////////

bool CSshKeysController::current_key_is_allselected() const {
  if (m_current_key.isEmpty()) return false;
  if (m_current_key_col < 0 ||
      m_current_key_col >= (int)m_lst_all_selected.size())
    return false;
  return m_lst_all_selected[m_current_key_col];
}
////////////////////////////////////////////////////////////////////////////

bool CSshKeysController::set_current_key_allselected(bool flag) {
  if (m_current_key.isEmpty()) return false;
  if (m_current_key_col < 0 || m_current_key_col >= (int)m_cols) return false;

  m_lst_all_selected[m_current_key_col] = flag;
  SynchroPrimitives::Locker lock(&csRbm);
  for (int32_t row = 0; row < m_rows; ++row) {
    m_current_ke_matrix[row][m_current_key_col] = flag ? 1 : 0;
  }
  return true;
}
////////////////////////////////////////////////////////////////////////////

QStringList CSshKeysController::keys_in_environment(
    const QString &env_id) const {
  QStringList result;
  int32_t row = 0;
  bool found = false;

  SynchroPrimitives::Locker lock(&csRbm);
  for (row = 0; row < (int32_t)m_lst_healthy_environments.size(); ++row) {
    if (m_lst_healthy_environments[row].id() != env_id) continue;
    found = true;
    break;
  }

  if (!found) return result;
  if (row >= m_rows) return result;

  for (int32_t col = 0; col < m_cols; ++col) {
    if (!m_current_ke_matrix[row][col]) continue;
    result << m_lst_key_files[col];
  }

  return result;
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::refresh_healthy_environments() {
  m_lst_healthy_environments =
      CHubController::Instance().lst_healthy_environments();
  QtConcurrent::run(this, &CSshKeysController::rebuild_bit_matrix);
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::ssh_key_send_progress_sl(int part, int total) {
  emit ssh_key_send_progress(part, total);
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::environments_updated(int rr) {
  if (rr == CHubController::RER_NO_DIFF || something_changed()) return;
  refresh_healthy_environments();
}
////////////////////////////////////////////////////////////////////////////

void CSshKeysController::refresh_files_timer_timeout() { refresh_key_files(); }
////////////////////////////////////////////////////////////////////////////
