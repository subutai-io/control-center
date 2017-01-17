#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>

#include <RestWorker.h>
#include <QFileDialog>

#include <QLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "DlgGenerateSshKey.h"
#include "ui_DlgGenerateSshKey.h"
#include "SystemCallWrapper.h"
#include "HubController.h"
#include "SettingsManager.h"
#include "NotifiactionObserver.h"

class SshKeyWidget : public QObject {
  Q_OBJECT
private:
  QString m_id;
  QLabel* m_lbl_name;
  QComboBox* m_combo_box;
  QPushButton* m_btn_add_to_environment;

public:
  SshKeyWidget(const CEnvironmentEx* env,
               QWidget* parent = nullptr) : QObject(parent), m_id(env->id()) {
    m_lbl_name = new QLabel(env->name(), parent);
  }

  virtual ~SshKeyWidget(){}

private slots:
  void btn_add_to_environment_released_sl();
signals:
  void btn_add_to_environment_released(const QString&);
};
////////////////////////////////////////////////////////////////////////////


DlgGenerateSshKey::DlgGenerateSshKey(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgGenerateSshKey)
{
  ui->setupUi(this);

  m_btn_generate = new QPushButton("Generate", this);
  m_layout_grid = new QGridLayout;
  m_layout_hbox = new QHBoxLayout;

  m_layout_grid->addWidget(m_btn_generate, 1, 1);

//  QPushButton** buttons = new QPushButton*[9];
//  int i, j;
//  for (i = 0, j = -1; i < 9; ++i) {
//    if (i % 3 == 0) ++j;
//    buttons[i] = new QPushButton(QString("ttt%1").arg(i), this);
//    m_layout_grid->addWidget(buttons[i], j, i%3);
//  }
  m_layout_grid->removeWidget(m_btn_generate);
  m_layout_grid->addWidget(m_btn_generate, 3, 0, 1, 3);

  m_layout_hbox->addLayout(m_layout_grid);
  this->setLayout(m_layout_hbox);

  connect(m_btn_generate, SIGNAL(released()), this, SLOT(btn_generate_released()));
}
////////////////////////////////////////////////////////////////////////////

DlgGenerateSshKey::~DlgGenerateSshKey() {
  if (m_layout_grid) m_layout_grid->deleteLater();
  if (m_layout_hbox) m_layout_hbox->deleteLater();
  if (m_btn_generate) m_btn_generate->deleteLater();
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::generate_new_ssh() {

  QString str_file = QFileDialog::getSaveFileName(this, "Generate new ssh key pair",
                                                  CSettingsManager::Instance().ssh_keys_storage());
  if (str_file.isEmpty()) return;

  QFile key(ssh_key_path(SP_PRIV_KEY));
  QFile key_pub(ssh_key_path(SP_PUB_KEY));

  if (key.exists() && key_pub.exists()) {
    key.remove();
    key_pub.remove();
  }

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::generate_ssh_key(CHubController::Instance().current_user(),
                                           ssh_key_path(SP_PRIV_KEY));
  if (scwe != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->NotifyAboutError(
          QString("Can't generate ssh-key. Err : %1").arg(CSystemCallWrapper::scwe_error_to_str(scwe)));
    return;
  }

  set_key_text();
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::set_key_text() {
  QString path = ssh_key_path(SP_PUB_KEY);
  QFile file(path);
  if (file.exists()) {
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    qDebug() << bytes;
  } else {
    QFile standard_key_file(ssh_key_path(SP_STANDARD_PUB_KEY));
    if (standard_key_file.exists()) {
      standard_key_file.open(QFile::ReadOnly);
      QByteArray bytes = standard_key_file.readAll();
      qDebug() << bytes;
    }
  }
}
////////////////////////////////////////////////////////////////////////////

QString
DlgGenerateSshKey::ssh_key_path(DlgGenerateSshKey::ssh_key_paths path) {
  QString base_path = CSettingsManager::Instance().ssh_keys_storage() +
                      QDir::separator();
  QString file_path[] = {CHubController::Instance().current_user(),
                         CHubController::Instance().current_user(),
                         "id_rsa", "id_rsa"};
  static QString ext[] = {".pub", "", ".pub", ""};
  return base_path + file_path[path] + ext[path];
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_generate_released() {
  QFileInfo fi(CSettingsManager::Instance().ssh_keys_storage());
  if (!fi.isDir() || !fi.isWritable()) {
    CNotificationObserver::Instance()->NotifyAboutInfo(
          "You don't have write permission to ssh-keys directory. Please change it in settings. Thanks");
    return;
  }

  //todo use some flag
  if (true) {
    generate_new_ssh();
  } else {
    QMessageBox *msg_question = new QMessageBox;
    do  {
      msg_question->setWindowTitle("New SSH key generation");
      msg_question->setText("Are you sure you want to generate new SSH key? You will need to update your environments");
      msg_question->addButton(QMessageBox::Yes);
      msg_question->addButton(QMessageBox::No);
      msg_question->setDefaultButton(QMessageBox::No);
      if (msg_question->exec() == QMessageBox::No)
        break;
      generate_new_ssh();
    } while (0);
    msg_question->deleteLater();
  }
}
////////////////////////////////////////////////////////////////////////////
