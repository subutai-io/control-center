#include "SshKeysController.h"

#include <QDir>
#include <QFile>
#include <RestWorker.h>
#include <QFileDialog>
#include <QThread>

#include "HubController.h"
#include "SystemCallWrapper.h"
#include "NotificationObserver.h"

std::vector<bool> CSshKeysController::empty_environments_bitmask;

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
  empty_environments_bitmask = std::vector<bool>(
                                 CHubController::Instance().lst_healthy_environments().size());

  for (auto i = CHubController::Instance().lst_healthy_environments().begin();
       i != CHubController::Instance().lst_healthy_environments().end(); ++i) {
    m_dct_environment_keyflags[i->id()] =
        CRestWorker::Instance()->is_sshkeys_in_environment(m_lst_key_content, i->id());
  }

  for (int i = 0; i < m_lst_key_files.size(); ++i) {
    m_dct_key_environments[m_lst_key_files[i]] =
        std::vector<bool>(CHubController::Instance().lst_healthy_environments().size());
    m_dct_key_allselected[m_lst_key_files[i]] = false;

    int k = 0;
    for (auto j = m_dct_environment_keyflags.begin();
         j != m_dct_environment_keyflags.end(); ++j, ++k) {
       m_dct_key_environments[m_lst_key_files[i]][k] = j->second[i];
    }
  }

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
CSshKeysController::send_data_to_hub() const {
  map_string_bitmask::const_iterator current = m_dct_key_environments.cbegin();
  map_string_bitmask::const_iterator original = m_dct_key_environments_original.cbegin();
  std::map<QString, std::vector<QString> > dct_to_send;
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

      if (dct_to_send.find(m_lst_key_content[k]) == dct_to_send.end())
        dct_to_send[m_lst_key_content[k]] = std::vector<QString>();

      dct_to_send[m_lst_key_content[k]].push_back(
        CHubController::Instance().lst_healthy_environments()[i].id());
    }
  }

  QThread* st = new QThread;
  SshControllerBackgroundWorker* bw =
      new SshControllerBackgroundWorker(dct_to_send, lst_key_names);

  connect(bw, &SshControllerBackgroundWorker::send_key_finished, st, &QThread::quit);
  connect(st, &QThread::started, bw, &SshControllerBackgroundWorker::start_send_keys_to_hub);
  connect(bw, &SshControllerBackgroundWorker::send_key_finished, st, &QThread::quit);
  connect(bw, &SshControllerBackgroundWorker::send_key_progress,
          this, &CSshKeysController::ssh_key_send_progress_sl);
  connect(st, &QThread::finished, st, &QThread::deleteLater);
  connect(st, &QThread::finished, bw, &SshControllerBackgroundWorker::deleteLater);
  bw->moveToThread(st);
  st->start();
}
////////////////////////////////////////////////////////////////////////////

void
CSshKeysController::set_key_environments_bit(int index, bool bit) {
  if (m_current_key.isEmpty()) return;
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
  UNUSED_ARG(rr);
  rebuild_bitmasks();
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

SshControllerBackgroundWorker::SshControllerBackgroundWorker(const std::map<QString, std::vector<QString> > &dct_key_environments,
    const std::vector<QString>& lst_key_names) :
  m_dct_key_environments(std::move(dct_key_environments)),
  m_lst_key_names(std::move(lst_key_names)) {
}

SshControllerBackgroundWorker::~SshControllerBackgroundWorker() {
}
////////////////////////////////////////////////////////////////////////////

void
SshControllerBackgroundWorker::start_send_keys_to_hub() {
  int part = 0;
  int total = (int) m_dct_key_environments.size();
  size_t j = 0;
  for (auto i = m_dct_key_environments.begin(); i != m_dct_key_environments.end(); ++i, ++j) {
    qDebug() << m_lst_key_names[j];
    qDebug() << i->first;
    CRestWorker::Instance()->add_sshkey_to_environments(m_lst_key_names[j],
                                                        i->first,
                                                        i->second);
    emit send_key_progress(++part, total);
  }

  emit send_key_finished();
}
////////////////////////////////////////////////////////////////////////////
