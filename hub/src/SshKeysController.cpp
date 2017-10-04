#include "SshKeysController.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <algorithm>
#include <QTimer>

#include "RestWorker.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"
#include "Locker.h"

CSshKeysController::CSshKeysController() :
  m_current_key_col(-1),
  m_refresh_files_timer(nullptr) {

  m_lst_healthy_environments = CHubController::Instance().lst_healthy_environments();
  m_refresh_files_timer = new QTimer;
  m_refresh_files_timer->setInterval(5000);

  refresh_key_files();
  rebuild_bit_matrix();

  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &CSshKeysController::environments_updated);
  connect(m_refresh_files_timer, &QTimer::timeout,
          this, &CSshKeysController::refresh_files_timer_timeout);
  m_refresh_files_timer->start();
}

CSshKeysController::~CSshKeysController() {
  if (m_refresh_files_timer) m_refresh_files_timer->deleteLater();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::refresh_key_files() {
  QStringList lst_key_content;
  QStringList lst_key_files;

  QDir dir(CSettingsManager::Instance().ssh_keys_storage());
  if (!dir.exists()) {
    CApplicationLog::Instance()->LogError("Wrong ssh keys storage");
    return;
  }

  QStringList name_filters({"*.pub"});
  QStringList tmp_list = dir.entryList(name_filters,
                                       QDir::Files | QDir::NoSymLinks);

  for (auto i = tmp_list.begin(); i != tmp_list.end(); ++i) {
    QString file_path = dir.path() + QDir::separator() + *i;
    QFile key_file(file_path);
    if (!key_file.open(QFile::ReadOnly)) {
      CApplicationLog::Instance()->LogError("Can't open ssh-key file : %s, reason : %s",
                                            file_path.toStdString().c_str(),
                                            key_file.errorString().toStdString().c_str());
      continue;
    }
    lst_key_files.push_back(*i);
    QByteArray arr_content = key_file.readAll();
    arr_content.truncate(arr_content.size() - 1); //hack for hub
    lst_key_content.push_back(QString(arr_content));
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
  rebuild_bit_matrix();
}
////////////////////////////////////////////////////////////////////////////

static SynchroPrimitives::CriticalSection cs_rbm;
void
CSshKeysController::rebuild_bit_matrix() {

  SynchroPrimitives::Locker lock(&cs_rbm);
  size_t cols, rows;
  std::vector<CEnvironment> lst_he = m_lst_healthy_environments; //copy because of wrong sync
  rows = m_lst_healthy_environments.size();
  cols = m_lst_key_files.size();
  m_current_bit_matrix.erase(m_current_bit_matrix.begin(), m_current_bit_matrix.end());

  m_current_bit_matrix.reserve(rows);
  m_current_bit_matrix.resize(rows);

  m_lst_all_selected.reserve(cols);
  m_lst_all_selected.resize(cols);

  for (size_t row = 0; row < rows; ++row) {
    m_current_bit_matrix[row] = std::vector<bool>(cols);
  }

  for (size_t row = 0; row < rows; ++row) {
    m_current_bit_matrix[row] = CRestWorker::Instance()->is_sshkeys_in_environment(m_lst_key_content,
                                  lst_he[row].id());
  }
  m_original_bit_matrix.erase(m_original_bit_matrix.begin(), m_original_bit_matrix.end());
  std::copy(m_current_bit_matrix.begin(), m_current_bit_matrix.end(),
            std::back_inserter(m_original_bit_matrix));

  emit matrix_updated();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::generate_new_ssh_key(QWidget* parent) {
  QString str_file = QFileDialog::getSaveFileName(parent, "Generate new ssh key pair. Don't change directory, please",
                                                  CSettingsManager::Instance().ssh_keys_storage(),
                                                  "Ssh keys (*.pub);; All files (*.*)");
  if (str_file.isEmpty()) return;
  QFileInfo fi(str_file);

  QString str_private = CSettingsManager::Instance().ssh_keys_storage() +
                        QDir::separator() + fi.baseName();

  QString str_public = str_file + (str_private == str_file ? ".pub" : "");

  QFile key(str_private);
  QFile key_pub(str_public);

  if (key.exists() && key_pub.exists()) {
    key.remove();
    key_pub.remove();
  }

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::generate_ssh_key(CHubController::Instance().current_user(),
                                           str_private);
  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error(
          QString("Can't generate ssh-key. Err : %1").
          arg(CSystemCallWrapper::scwe_error_to_str(scwe)));
    return;
  }
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::send_data_to_hub() {
  std::map<QString, std::pair<QString, std::vector<QString> > > dct_to_send;
  std::map<QString, std::pair<QString, std::vector<QString> > > dct_to_remove;
  size_t cols, rows;
  std::vector<size_t> lst_to_remove_idxs;
  std::vector<CEnvironment> lst_to_leave;

  {
    SynchroPrimitives::Locker lock(&cs_rbm);
    cols = m_lst_key_files.size();
    rows = m_lst_healthy_environments.size();
    std::vector<CEnvironment> lst_he = m_lst_healthy_environments;

    for (size_t col = 0; col < cols; ++col) {
      QString key = m_lst_key_content[col];
      QString key_name = m_lst_key_files[col];

      for (size_t row = 0; row < rows; ++row) {
        if (m_current_bit_matrix[row][col] == m_original_bit_matrix[row][col]) {
          continue;
        }

        if (std::find(lst_to_remove_idxs.begin(), lst_to_remove_idxs.end(), row) == lst_to_remove_idxs.end()) {
          lst_to_remove_idxs.push_back(row);
        }

        if (!m_current_bit_matrix[row][col]) {
          dct_to_remove[key].first = key_name;
          dct_to_remove[key].second.push_back(lst_he[row].id());
        } else {
          dct_to_send[key].first = key_name;
          dct_to_send[key].second.push_back(lst_he[row].id());
        }
      } //for row
    } //for col

    for (size_t i = 0; i < lst_he.size(); ++i) {
      if (std::find(lst_to_remove_idxs.begin(), lst_to_remove_idxs.end(), i) != lst_to_remove_idxs.end()) continue;
      lst_to_leave.push_back(lst_he[i]);
    }

    m_lst_healthy_environments = lst_to_leave;
  }

  int part = 0;
  int total = (int) (dct_to_send.size() + dct_to_remove.size());
  for (auto i = dct_to_send.begin(); i != dct_to_send.end(); ++i) {
    CRestWorker::Instance()->add_sshkey_to_environments(i->second.first,
                                                        i->first,
                                                        i->second.second);
    emit ssh_key_send_progress(++part, total);
  }

  for (auto i = dct_to_remove.begin(); i != dct_to_remove.end(); ++i) {
    CRestWorker::Instance()->remove_sshkey_from_environments(i->second.first,
                                                             i->first,
                                                             i->second.second);
    emit ssh_key_send_progress(++part, total);
  }

  emit ssh_key_send_finished();
  rebuild_bit_matrix();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::set_current_key(const QString &key) {
  m_current_key_col = m_lst_key_files.indexOf(key);
  if (m_current_key_col != -1) m_current_key = key;
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::set_key_environments_bit(int index, bool bit) {
  if (m_current_key.isEmpty()) return;
  if (index < 0 || index >= (int)m_current_bit_matrix.size()) return;
  m_current_bit_matrix[index][m_current_key_col] = bit;

  bit = true;
  for (size_t row = 0; row < m_current_bit_matrix.size(); ++row) {
    if (m_current_bit_matrix[row][m_current_key_col])
      continue;
    bit = false;
    break;
  }
  m_lst_all_selected[m_current_key_col] = bit;
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::get_key_environments_bit(int index) const {
  if (m_current_key.isEmpty()) return false;
  if (index < 0 || index >= (int)m_current_bit_matrix.size()) return false;
  if (m_current_bit_matrix[index].empty() || m_current_key_col >= (int)m_current_bit_matrix[index].size())
    return false;
  return m_current_bit_matrix[index][m_current_key_col];
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::current_key_is_allselected() const {
  if (m_current_key.isEmpty()) return false;
  if (m_current_key_col < 0 || m_current_key_col >= (int)m_lst_all_selected.size()) return false;
  return m_lst_all_selected[m_current_key_col];
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::set_current_key_allselected(bool flag) {
  if (m_current_key.isEmpty()) return false;
  if (m_current_key_col < 0 || m_current_key_col >= (int)m_lst_all_selected.size()) return false;

  m_lst_all_selected[m_current_key_col] = flag;
  for (size_t row = 0; row < m_current_bit_matrix.size(); ++row) {
    m_current_bit_matrix[row][m_current_key_col] = flag;
  }
  return true;
}
////////////////////////////////////////////////////////////////////////////

QStringList
CSshKeysController::keys_in_environment(const QString &env_id) const {
  QStringList result;
  size_t row = 0;
  bool found = false;
  for (row = 0; row < m_lst_healthy_environments.size(); ++row) {
    if (m_lst_healthy_environments[row].id() != env_id) continue;
    found = true;
    break;
  }

  if (!found) return result;

  for (int col = 0; col < m_lst_key_files.size(); ++col) {
    if (!m_current_bit_matrix[row][col]) continue;
    result << m_lst_key_files[col];
  }

  return result;
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::refresh_healthy_environments() {
  static SynchroPrimitives::CriticalSection cs;
  SynchroPrimitives::Locker lock(&cs);
  m_lst_healthy_environments = CHubController::Instance().lst_healthy_environments();
  rebuild_bit_matrix();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::ssh_key_send_progress_sl(int part, int total) {
  emit ssh_key_send_progress(part, total);
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::environments_updated(int rr) {
  if (rr == CHubController::RER_NO_DIFF || something_changed()) return;
  refresh_healthy_environments();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::refresh_files_timer_timeout() {
  refresh_key_files();
}
////////////////////////////////////////////////////////////////////////////
