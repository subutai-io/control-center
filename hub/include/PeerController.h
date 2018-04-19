#ifndef PEERCONTROLLER_H
#define PEERCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <map>
#include "SystemCallWrapper.h"
#include <QFileSystemModel>
#include "RestContainers.h"
#include <QtConcurrent/QtConcurrent>

class CLocalPeer {

private:
    QString m_name;
    QString m_ip;
    QString m_fingerprint;
    QString m_status;
    QString m_update;
    QString m_dir;
public:
    CLocalPeer(){
        m_name = "loading";
        m_ip = "loading";
        m_status = "loading";
        m_dir = "loading";
        m_fingerprint = "loading";
        m_update = "old";
    }
    const QString &ip() const { return m_ip; }
    const QString &name() const { return m_name; }
    const QString &fingerprint() const { return m_fingerprint; }
    const QString &status() const { return m_status; }
    const QString &dir() const { return m_dir; }
    const QString &update() const { return m_update; }

    void set_ip(const QString &val){m_ip = val;}
    void set_name(const QString &val){m_name = val;}
    void set_status(const QString &val){m_status = val;}
    void set_dir(const QString &val){m_dir = val;}
    void set_fingerprint(const QString &val){m_fingerprint = val;}
    void set_update(const QString &val){m_update = val;}

    void operator =(const CLocalPeer &peer_second){
        this->m_name = peer_second.name();
        this->m_ip = peer_second.ip();
        this->m_dir = peer_second.dir();
        this->m_fingerprint = peer_second.fingerprint();
        this->m_status = peer_second.status();
        this->m_update = peer_second.update();
    }

};

class CPeerController : public QObject {
  Q_OBJECT

private:
  CPeerController(QObject* parent = nullptr);
  virtual ~CPeerController();

  QTimer m_refresh_timer;
  QTimer m_logs_timer;


  void search_local();
  void check_logs();
  QString get_error_messages(QDir peer_dir, QString command);
  void get_peer_info(const QFileInfo &fi, QDir dir);
  QString parse_name(const QString &name);
  int number_threads; // to emit signal when all finished

public:

  static CPeerController* Instance() {
    static CPeerController inst;
    return &inst;
  }

  void init();
  void refresh();
  void parse_peer_info(int type,
                       const QString &name,
                       const QString &dir,
                       const QString &output);

  int get_number_threads(){
      return number_threads;
  }

  void dec_number_threads(){
      if(number_threads > 0)
        number_threads--;
  }

  void finish_current_update(){
      number_threads = 0;
  }


private slots:
  void refresh_timer_timeout();
signals:
  void got_peer_info(int type,
                     QString name,
                     QString dir,
                     QString output);
};

class GetPeerInfo : public QObject{
    Q_OBJECT
    QString arg;
    int action;
public:
    GetPeerInfo(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &arg, const int &action){
        this->arg = arg;
        this->action = action;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &GetPeerInfo::execute_remote_command);
        connect(this, &GetPeerInfo::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &GetPeerInfo::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<QString> *watcher
            = new QFutureWatcher<QString>(this);
        QFuture<QString> res;
        switch (action) {
        case 0:
            res = QtConcurrent::run(CSystemCallWrapper::vagrant_status, arg);
            break;
        case 1:
            res = QtConcurrent::run(CSystemCallWrapper::vagrant_port, arg);
            break;
        case 2:
            res = QtConcurrent::run(CSystemCallWrapper::vagrant_fingerprint, arg);
            break;
        }
        watcher->setFuture(res);
        int lala = action;
        connect(watcher, &QFutureWatcher<QString>::finished, [this, res, lala](){
          emit this->outputReceived(lala, res.result());
        });
    }
signals:
    void outputReceived(int action, QString res);
};

class StopPeer : public QObject{
    Q_OBJECT
    QString directory;
public:
    StopPeer(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &StopPeer::execute_remote_command);
        connect(this, &StopPeer::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &StopPeer::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture<system_call_wrapper_error_t>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_halt, directory);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
          emit this->outputReceived(res);
        });
    }
signals:
    void outputReceived(system_call_wrapper_error_t res);
};

class CommandPeerTerminal : public QObject{
    Q_OBJECT
    QString directory;
    QString command;
    QString name;
public:
    CommandPeerTerminal(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &dir, const QString &command, const QString &name){
        this->directory = dir;
        this->command = command;
        this->name = name;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &CommandPeerTerminal::execute_remote_command);
        connect(this, &CommandPeerTerminal::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &CommandPeerTerminal::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture<system_call_wrapper_error_t>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_command_terminal, directory, command, name);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
          emit this->outputReceived(res);
        });
    }
signals:
    void outputReceived(system_call_wrapper_error_t res);
};

class DestroyPeer : public QObject{
    Q_OBJECT
    QString directory;
public:
    DestroyPeer(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &DestroyPeer::execute_remote_command);
        connect(this, &DestroyPeer::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &DestroyPeer::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture<system_call_wrapper_error_t>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_destroy, directory);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
          emit this->outputReceived(res);
        });
    }
signals:
    void outputReceived(system_call_wrapper_error_t res);
};

class ReloadPeer : public QObject{
    Q_OBJECT
    QString directory;
public:
    ReloadPeer(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &ReloadPeer::execute_remote_command);
        connect(this, &ReloadPeer::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &ReloadPeer::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture<system_call_wrapper_error_t>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_reload, directory);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
          emit this->outputReceived(res);
        });
    }
signals:
    void outputReceived(system_call_wrapper_error_t res);
};

#endif
