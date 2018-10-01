#ifndef PEERCONTROLLER_H
#define PEERCONTROLLER_H

#include <QFileSystemModel>
#include <QObject>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <map>
#include <set>
#include "RestContainers.h"
#include "SystemCallWrapper.h"

class CLocalPeer {
 private:
  QString m_name;
  QString m_ip;
  QString m_fingerprint;
  QString m_status;
  QString m_update;
  QString m_dir;
  QString m_update_available;
  int m_provision_step;

 public:
  CLocalPeer() {
    m_provision_step = -1;
    m_name = "undefined";
    m_ip = "undefined";
    m_status = "undefined";
    m_dir = "undefined";
    m_fingerprint = "undefined";
    m_update = "old";
    m_update_available = "false";
  }
  const QString &ip() const { return m_ip; }
  const QString &name() const { return m_name; }
  const QString &fingerprint() const { return m_fingerprint; }
  const QString &status() const { return m_status; }
  const QString &dir() const { return m_dir; }
  const QString &update() const { return m_update; }
  const QString &update_available() const { return m_update_available; }
  const int &provision_step() const { return m_provision_step; }

  void set_ip(const QString &val) { m_ip = val; }
  void set_name(const QString &val) { m_name = val; }
  void set_status(const QString &val) { m_status = val; }
  void set_dir(const QString &val) { m_dir = val; }
  void set_fingerprint(const QString &val) { m_fingerprint = val; }
  void set_update(const QString &val) { m_update = val; }
  void set_update_available(const QString &val) {
    if (m_update_available == "updating") {
      if (val == "finished") {
        m_update_available = "false";
      } else {
        return;
      }
    } else {
      m_update_available = val;
    }
  }
  void set_provision_step(const int &val) { m_provision_step = val; }

  void operator=(const CLocalPeer &peer_second) {
    this->m_name = peer_second.name();
    this->m_ip = peer_second.ip();
    this->m_dir = peer_second.dir();
    this->m_fingerprint = peer_second.fingerprint();
    this->m_status = peer_second.status();
    this->m_update = peer_second.update();
    this->m_update_available = peer_second.update_available();
    this->m_provision_step = peer_second.provision_step();
  }
};

class CPeerController : public QObject {
  Q_OBJECT

 private:
  CPeerController(QObject *parent = nullptr);
  virtual ~CPeerController();

  QTimer m_refresh_timer;
  QTimer m_logs_timer;
  std::set<QString> checking_statutes; // Saves peer cheking status, key: peer name, value: peer directory)

  void insert_checking_status(QString dir) {
    qDebug() << "LOCKED PEER STATE: "
             << QDir::toNativeSeparators(dir);
    checking_statutes.insert(QDir::toNativeSeparators(dir));
  }

  void remove_checking_status(QString dir) {
    std::set<QString>::iterator ite;
    ite = checking_statutes.find(QDir::toNativeSeparators(dir));
    if (ite != checking_statutes.end()) {
      checking_statutes.erase(ite);
      qDebug() << "UNLOCKED PEER STATE: "
               << QDir::toNativeSeparators(dir);
    }
  }

  void search_local();
  void check_logs();
  QString get_error_messages(QDir peer_dir, QString command);
  void get_peer_info(const QFileInfo &fi, QDir dir);
  QString parse_name(const QString &name);

  QStringList bridged_interfaces;  // list of current bridged interfaces
  system_call_res_t vagrant_global_status;  // list of current VMs directory(Only Subutai peers)
  int number_threads;                       // to emit signal when all finished
  QString get_pr_step_fi(QFile &p_file); // get provision step from file
  bool is_provision_running(QDir peer_dir);

 public:
  enum peer_info_t {
    P_STATUS = 0,
    P_PORT,
    P_FINGER,
    P_UPDATE,
    P_PROVISION_STEP
  };

  static CPeerController *Instance() {
    static CPeerController inst;
    return &inst;
  }

  void init();
  void refresh();
  void parse_peer_info(peer_info_t type, const QString &name,
                       const QString &dir, const QString &output);

  int get_number_threads() { return number_threads; }

  void dec_number_threads() {
    if (number_threads > 0) number_threads--;
  }

  void finish_current_update() { number_threads = 0; }

  QStringList get_bridgedifs() { return bridged_interfaces; }
  system_call_res_t get_global_status() { return vagrant_global_status; }
  const QString &status_description(const QString &status);
  const QString &provision_step_description(const int &step);
  int getProvisionStep(const QString &dir);

  bool is_checking_status(QString dir) {
    if (checking_statutes.count(dir)) {
      return true;
    }

    return false;
  }

 private slots:
  void refresh_timer_timeout();
 signals:
  void got_peer_info(peer_info_t type, QString name, QString dir,
                     QString output);
};

class GetPeerInfo : public QObject {
  Q_OBJECT
  QString arg;
  CPeerController::peer_info_t action;

 public:
  GetPeerInfo(QObject *parent = nullptr) : QObject(parent) {}

  void init(const QString &arg, const CPeerController::peer_info_t &action) {
    this->arg = arg;
    this->action = action;
  }

  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this,
            &GetPeerInfo::execute_remote_command);
    connect(this, &GetPeerInfo::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &GetPeerInfo::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }

  void execute_remote_command() {
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>(this);
    QFuture<QString> res;
    switch (action) {
      case 0:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_status, arg);
        break;
      case 1:
        res = QtConcurrent::run(CSystemCallWrapper::vagrant_port, arg);
        break;
      default:
        break;
    }
    watcher->setFuture(res);
    CPeerController::peer_info_t lala = action;
    connect(watcher, &QFutureWatcher<QString>::finished, [this, res, lala]() {
      emit this->outputReceived(lala, res.result());
    });
  }
 signals:
  void outputReceived(CPeerController::peer_info_t action, QString res);
};

class StopPeer : public QObject {
  Q_OBJECT
  QString directory;

 public:
  StopPeer(QObject *parent = nullptr) : QObject(parent) {}

  void init(const QString &directory) { this->directory = directory; }

  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this, &StopPeer::execute_remote_command);
    connect(this, &StopPeer::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &StopPeer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }

  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::vagrant_halt, directory);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            watcher, &QFutureWatcher<system_call_wrapper_error_t>::deleteLater);
  }
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};

class CommandPeerTerminal : public QObject {
  Q_OBJECT
  QString directory;
  QString command;
  QString name;

 public:
  CommandPeerTerminal(QObject *parent = nullptr) : QObject(parent) {}

  void init(const QString &dir, const QString &command, const QString &name) {
    this->directory = dir;
    this->command = command;
    this->name = name;
  }

  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this,
            &CommandPeerTerminal::execute_remote_command);
    connect(this, &CommandPeerTerminal::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this,
            &CommandPeerTerminal::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }

  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res = QtConcurrent::run(
        CSystemCallWrapper::vagrant_command_terminal, directory, command, QString("\"%1\"").arg(name));
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::finished,
        watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::deleteLater);
  }
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};

class DestroyPeer : public QObject {
  Q_OBJECT
  QString directory;

 public:
  DestroyPeer(QObject *parent = nullptr) : QObject(parent) {}

  void init(const QString &directory) { this->directory = directory; }

  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this,
            &DestroyPeer::execute_remote_command);
    connect(this, &DestroyPeer::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &DestroyPeer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }

  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::vagrant_destroy, directory);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::finished,
        watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::deleteLater);
  }
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};

class ReloadPeer : public QObject {
  Q_OBJECT
  QString directory;

 public:
  ReloadPeer(QObject *parent = nullptr) : QObject(parent) {}

  void init(const QString &directory) { this->directory = directory; }

  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this,
            &ReloadPeer::execute_remote_command);
    connect(this, &ReloadPeer::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &ReloadPeer::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }

  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res =
        QtConcurrent::run(CSystemCallWrapper::vagrant_reload, directory);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::finished,
        watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::deleteLater);
  }
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};

/////////////*this class updates information of bridged interfaces and global
///status of all VMs*////////////////////
class UpdateVMInformation : public QObject {
  Q_OBJECT
 public:
  UpdateVMInformation(QObject *parent = nullptr) : QObject(parent) {}
  void startWork() {
    QThread *thread = new QThread();
    connect(thread, &QThread::started, this,
            &UpdateVMInformation::execute_remote_command);
    connect(this, &UpdateVMInformation::outputReceived, thread, &QThread::quit);
    connect(thread, &QThread::finished, this,
            &UpdateVMInformation::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }
  void execute_remote_command() {
    QFutureWatcher<std::pair<QStringList, system_call_res_t> > *watcher =
        new QFutureWatcher<std::pair<QStringList, system_call_res_t> >(this);
    QFuture<std::pair<QStringList, system_call_res_t> > res =
        QtConcurrent::run(CSystemCallWrapper::vagrant_update_information);
    watcher->setFuture(res);
    connect(
        watcher,
        &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::finished,
        [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::finished,
        watcher, &QFutureWatcher<std::pair<QStringList, system_call_res_t> >::deleteLater);
  }
 signals:
  void outputReceived(std::pair<QStringList, system_call_res_t> res);
};

// updates rh and management of peer
class UpdatePeerOS : public QObject {
  Q_OBJECT
public:
  UpdatePeerOS(QObject *parent = nullptr) : QObject(parent) {}
  void startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &UpdatePeerOS::execute_remote_command);
    connect(this, &UpdatePeerOS::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &UpdatePeerOS::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }
  void init(const QString name, const QString port) {
    m_peer_name = name;
    m_peer_port = port;
  }
  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher =
        new QFutureWatcher<system_call_wrapper_error_t>(this);
    QFuture<system_call_wrapper_error_t> res = QtConcurrent::run(
        CSystemCallWrapper::vagrant_update_peeros, m_peer_port, m_peer_name);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            [this, res]() { emit this->outputReceived(res); });
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished,
            watcher, &QFutureWatcher<system_call_wrapper_error_t>::deleteLater);
  }

 private:
  QString m_peer_name;
  QString m_peer_port;
 signals:
  void outputReceived(system_call_wrapper_error_t res);
};
#endif
