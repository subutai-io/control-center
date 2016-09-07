#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QClipboard>
#include <RestWorker.h>

#include "DlgGenerateSshKey.h"
#include "ui_DlgGenerateSshKey.h"
#include "SystemCallWrapper.h"
#include "HubController.h"
#include "SettingsManager.h"
#include "NotifiactionObserver.h"

DlgGenerateSshKey::DlgGenerateSshKey(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgGenerateSshKey)
{
  ui->setupUi(this);
  set_key_text();

  connect(ui->btn_generate, SIGNAL(released()), this, SLOT(btn_generate_released()));
  connect(ui->btn_copy_to_clipboard, SIGNAL(released()),
          this, SLOT(btn_copy_to_clipboard_released()));
  connect(ui->btn_add_to_environments, SIGNAL(released()),
          this, SLOT(btn_add_to_environments_released()));
}

DlgGenerateSshKey::~DlgGenerateSshKey() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::generate_new_ssh() {

  QFile key(ssh_private_key_path());
  QFile key_pub(ssh_pub_key_path());

  if (key.exists() && key_pub.exists()) {
    key.remove();
    key_pub.remove();
  }

  system_call_wrapper_error_t scwe =
      CSystemCallWrapper::generate_ssh_key(CHubController::Instance().current_user().toStdString().c_str(),
                                           ssh_private_key_path().toStdString().c_str());
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
  QString path = ssh_pub_key_path();
  QFile file(path);
  if (file.exists()) {
    file.open(QFile::ReadOnly);
    QByteArray bytes = file.readAll();
    ui->te_ssh_key->setText(QString(bytes));
  }
}
////////////////////////////////////////////////////////////////////////////

QString
DlgGenerateSshKey::ssh_pub_key_path() const {
  QString path = CSettingsManager::Instance().ssh_keys_storage() +
                 QDir::separator() +
                 CHubController::Instance().current_user() + ".pub";
  return path;
}
////////////////////////////////////////////////////////////////////////////

QString
DlgGenerateSshKey::ssh_private_key_path() const {
  QString path = CSettingsManager::Instance().ssh_keys_storage() +
                 QDir::separator() +
                 CHubController::Instance().current_user();
  return path;
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

  if (ui->te_ssh_key->toPlainText().isEmpty()) {
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

void
DlgGenerateSshKey::btn_copy_to_clipboard_released() {
  QString text = ui->te_ssh_key->toPlainText();
  if (text.isEmpty()) return;
  ui->te_ssh_key->selectAll();
  QApplication::clipboard()->setText(text, QClipboard::Clipboard);
}
////////////////////////////////////////////////////////////////////////////

void
DlgGenerateSshKey::btn_add_to_environments_released() {
  QString text = ui->te_ssh_key->toPlainText();
  if (text.isEmpty()) return;
  int http_code, err_code, network_err;
  CRestWorker::Instance()->send_ssh_key(text, http_code, err_code, network_err);
  CApplicationLog::Instance()->LogTrace("send ssh results : %d %d %d",
                                        http_code, err_code, network_err);
  QMessageBox *msg = new QMessageBox(this);
  if (err_code == RE_SUCCESS) {
    msg->setWindowTitle("Success");
    msg->setText("Operation successfuly completed");
    msg->addButton(QMessageBox::Yes);
  } else {
    msg->setWindowTitle("Operation failed");
    msg->setText("Add ssh to environments failed");
    msg->addButton(QMessageBox::Yes);
  }
  msg->exec();
  delete msg;
}
////////////////////////////////////////////////////////////////////////////
