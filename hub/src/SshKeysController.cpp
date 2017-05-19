#include "SshKeysController.h"

#include <QDir>
#include <QFile>
#include <RestWorker.h>
#include <QFileDialog>
#include <QThread>

#include "HubController.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"

CSshKeysController::CSshKeysController() {
  refresh_key_files();
  rebuild_bitmasks();
  connect(&CHubController::Instance(), &CHubController::environments_updated,
          this, &CSshKeysController::environments_updated);
}

CSshKeysController::~CSshKeysController() {

}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::refresh_key_files() {
  m_lst_key_content.clear();
  m_lst_key_files.clear();

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
    m_lst_key_files.push_back(*i);
    QByteArray arr_content = key_file.readAll();
    arr_content.truncate(arr_content.size() - 1); //hack for hub
    m_lst_key_content.push_back(QString(arr_content));
    key_file.close();
  }

  if (!m_lst_key_files.empty() && m_current_key.isEmpty())
    m_current_key = m_lst_key_files[0];

  rebuild_bitmasks();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::rebuild_bitmasks() {
  size_t healthy_environments_count = CHubController::Instance().lst_healthy_environments().size();
  size_t cols, rows;
  rows = healthy_environments_count;
  cols = m_lst_key_files.size();
  uint8_t **bit_map = new uint8_t*[rows];
  for (size_t row = 0; row < rows; ++row) {
    bit_map[row] = new uint8_t[m_lst_key_files.size()];
    memset(bit_map[row], 0, cols*sizeof(uint8_t));
  }

  for (size_t row = 0; row < rows; ++row) {
    std::vector<bool> tmp =
        CRestWorker::Instance()->
        is_sshkeys_in_environment(m_lst_key_content,
                                  CHubController::Instance().lst_healthy_environments()[row].id());
    for (size_t col = 0; col < cols; ++col) {
      if (!tmp[col]) continue;
      bit_map[row][col] = 1;
    }
  }

  //fill dct_environment_keyflags
  for (size_t row = 0; row < rows; ++row) {
    QString env_id = CHubController::Instance().lst_healthy_environments()[row].id();
    if (m_dct_environment_keyflags.find(env_id) == m_dct_environment_keyflags.end())
      m_dct_environment_keyflags[env_id] = std::vector<bool>(cols);

    for (size_t col = 0; col < cols; ++col) {
      m_dct_environment_keyflags[env_id][col] = bit_map[row][col] != 0;
    }
  }

  //fill dct_key_environments
  for (size_t col = 0; col < cols; ++col) {
    QString key_file = m_lst_key_files[col];
    m_dct_key_environments[key_file] = std::vector<bool>(rows);
    m_dct_key_allselected[key_file] = false;
    for (size_t row = 0; row < rows; ++row) {
      m_dct_key_environments[key_file][row] = bit_map[row][col] != 0;
    }
  }

  for (size_t row = 0; row < rows; ++row)
    delete [] bit_map[row];
  delete [] bit_map;

  m_dct_key_environments_original.insert(m_dct_key_environments.begin(),
                                         m_dct_key_environments.end());
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
  map_string_bitmask::const_iterator current = m_dct_key_environments.cbegin();
  map_string_bitmask::const_iterator original = m_dct_key_environments_original.cbegin();

  std::map<QString, std::pair<QString, std::vector<QString> > > dct_to_send;
  std::vector<QString> lst_key_names;

  for (size_t k = 0; current != m_dct_key_environments.cend();
       ++current, ++original, ++k) {
    bool contains = false;
    for (size_t i = 0; i < current->second.size(); ++i) {
      if (!current->second[i] ||
          current->second[i] == original->second[i]) continue;

      if (!contains) {
        lst_key_names.push_back(current->first);
        contains = true;
      }

      if (dct_to_send.find(m_lst_key_content[k]) == dct_to_send.end()) {
        dct_to_send[m_lst_key_content[k]].second = std::vector<QString>();
      }

      dct_to_send[m_lst_key_content[k]].first = lst_key_names[k];
      dct_to_send[m_lst_key_content[k]].second.push_back(
            CHubController::Instance().lst_healthy_environments()[i].id());
    }
  }

  int part = 0;
  int total = (int) dct_to_send.size();
  for (auto i = dct_to_send.begin(); i != dct_to_send.end(); ++i) {
    CRestWorker::Instance()->add_sshkey_to_environments(i->second.first,
                                                        i->first,
                                                        i->second.second);
    emit ssh_key_send_progress(++part, total);
  }
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::set_key_environments_bit(int index, bool bit) {
  if (m_current_key.isEmpty()) return;
  if (m_dct_key_environments.find(m_current_key) == m_dct_key_environments.end()) return;
  if (m_dct_key_environments[m_current_key].empty()) return;
  m_dct_key_environments[m_current_key][index] = bit;

  bit = true;
  for (size_t i = 0; i < m_dct_key_environments[m_current_key].size(); ++i) {
    if (m_dct_key_environments[m_current_key][i]) continue;
    bit = false;
  }
  m_dct_key_allselected[m_current_key] = bit;
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::get_key_environments_bit(int index) {
  if (m_current_key.isEmpty()) return false;
  if (m_dct_key_environments.find(m_current_key) ==
      m_dct_key_environments.end()) return false;
  if (m_dct_key_environments[m_current_key].empty()) return false;
  return m_dct_key_environments[m_current_key][index];
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::current_key_is_allselected() const {
  return m_dct_key_allselected.find(m_current_key) !=
      m_dct_key_allselected.end() && m_dct_key_allselected.at(m_current_key);
}
////////////////////////////////////////////////////////////////////////////

bool
CSshKeysController::set_current_key_allselected(bool flag) {
  if (m_dct_key_allselected.find(m_current_key) == m_dct_key_allselected.end())
    return false;
  if (m_dct_key_environments.find(m_current_key) == m_dct_key_environments.end())
    return false;

  for (size_t i = 0;  i < m_dct_key_environments[m_current_key].size(); ++i) {
    m_dct_key_environments[m_current_key][i] =
        m_dct_key_environments_original[m_current_key][i] || flag;
  }
  return true;
}
////////////////////////////////////////////////////////////////////////////

QStringList
CSshKeysController::keys_in_environment(const QString &env_id) const {
  QStringList result;
  if (m_dct_environment_keyflags.find(env_id) == m_dct_environment_keyflags.end())
    return result;

  for (size_t i = 0; i < m_dct_environment_keyflags.at(env_id).size(); ++i) {
    if (!m_dct_environment_keyflags.at(env_id)[i]) continue;
    result << m_lst_key_files[i];
  }
  return result;
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::ssh_key_send_progress_sl(int part, int total) {
  emit ssh_key_send_progress(part, total);
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::environments_updated(int rr) {
  if (rr == CHubController::RER_EMPTY ||
      rr == CHubController::RER_NO_DIFF) return;
  rebuild_bitmasks();
}
////////////////////////////////////////////////////////////////////////////
