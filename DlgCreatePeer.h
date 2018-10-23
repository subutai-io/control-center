#ifndef DLGCREATEPEER_H
#define DLGCREATEPEER_H

#include <QDebug>
#include <QDialog>
#include <QObject>
#include <QtConcurrent/QtConcurrent>

#include "DlgNotification.h"
#include "SystemCallWrapper.h"

namespace Ui {
class DlgCreatePeer;
}
// This structure 'requirement' needed to check requirements for peer creataion
// how to use:
// status label = 'Checking the VirtualBox'
// error_notification = 'VirtualBox is not installed please install it from
// Components' notification_type = N_N_ABOUT error_label: 'virtualbox is not
// istalled'
struct requirement {
  QString error_label;
  QString status_label;
  QString error_notification;
  DlgNotification::NOTIFICATION_ACTION_TYPE notification_type;
  bool (*checker_function)();
  requirement(QString _error_label, QString _status_label,
              QString _error_notification,
              DlgNotification::NOTIFICATION_ACTION_TYPE _notification_type,
              bool (*_checker_function)()) {
    error_label = _error_label;
    status_label = _status_label;
    error_notification = _error_notification;
    notification_type = _notification_type;
    checker_function = _checker_function;
  }
  ~requirement() {}
};

class DlgCreatePeer : public QDialog {
  Q_OBJECT

 public:
  explicit DlgCreatePeer(QWidget *parent = 0);
  ~DlgCreatePeer();
  QString create_dir(const QString &name);
  QString virtualbox_dir(const QString &name);
  QString vmware_dir(const QString &name);
  enum pass_err {
    PASS_EMPTY = 0,  // when empty password
    PASS_SMALL,      // when too small
    PASS_INVALID,    // when contains invalid symbols
    PASS_FINE
  };
  bool m_password_state, m_password_confirm_state;
  const QString BASE_PEER_FOLDER = "Subutai-peers";

 private:
  Ui::DlgCreatePeer *ui;
  QAction *m_show_confirm_password_action;
  QAction *m_show_password_action;
  pass_err check_pass(QString pass);
  void hide_err_labels();
  void set_enabled_buttons(bool state);
  void init_completed(system_call_wrapper_error_t res, QString dir, QString ram,
                      QString cpu, QString disk, int port);
  bool check_configurations();
  bool check_machine();
  int reserve_new_port();

  QRegExp m_invalid_chars;
  std::vector<requirement> m_requirements_ls;

 public slots:
  void create_button_pressed();
};

// init peer  executer
class InitPeer : public QObject {
  Q_OBJECT
  QString directory;
  QString OS;

 public:
  InitPeer(QObject *parent = nullptr) : QObject(parent) {}
  void init(const QString &directory, const QString &OS) {
    this->directory = directory;
    this->OS = OS;
  }
  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this, &InitPeer::execute_remote_command);
    connect(this, &InitPeer::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &InitPeer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }
  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::vagrant_init, directory, OS);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res.result()); });
  }
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};
#endif  // DLGCREATEPEER_H
