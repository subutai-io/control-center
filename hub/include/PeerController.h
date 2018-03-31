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
    QString m_dir;
public:
    explicit CLocalPeer(const QString &name, const QString &ip, const QString &fingerprint, const QString &status, const QString &dir){
        m_name = name;
        m_ip = ip;
        m_fingerprint = fingerprint;
        m_status = status;
        m_dir = dir;
    }
    const QString &ip() const { return m_ip; }
    const QString &name() const { return m_name; }
    const QString &fingerprint() const { return m_fingerprint; }
    const QString &status() const { return m_status; }
    const QString &dir() const { return m_dir; }
};

class CPeerController : public QObject {
  Q_OBJECT

private:
  CPeerController(QObject* parent = nullptr);
  virtual ~CPeerController();

  std::vector<CLocalPeer> m_local_peers;

  QTimer m_refresh_timer;

  int number_threads; // to emit signal when all finished

  void search_local();
  void get_peer_info(const QFileInfo &fi, QDir dir);
  QString parse_name(const QString &name);

public:

  static const int REFRESH_DELAY_SEC = 8;
  static CPeerController* Instance() {
    static CPeerController inst;
    return &inst;
  }

  void init();
  void refresh();
  void parse_peer_info(const QString &name, const QString &dir, const QStringList &output);

  const std::vector<CLocalPeer>& local_peers() const {
      return m_local_peers;
  }

  int get_number_threads(){
      return number_threads;
  }

private slots:
  void refresh_timer_timeout();
signals:
  void local_peer_list_updated();
};

class GetPeerInfo : public QObject{
    Q_OBJECT
    QString directory;
public:
    GetPeerInfo(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
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
        QFutureWatcher<QStringList> *watcher
            = new QFutureWatcher<QStringList>(this);
        QFuture<QStringList>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_peer_info, directory);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<QStringList>::finished, [this, res](){
          emit this->outputReceived(res.result());
        });
    }
signals:
    void outputReceived(QStringList res);
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

class StartPeer : public QObject{
    Q_OBJECT
    QString directory;
public:
    StartPeer(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &StartPeer::execute_remote_command);
        connect(this, &StartPeer::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &StartPeer::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<system_call_wrapper_error_t> *watcher
            = new QFutureWatcher<system_call_wrapper_error_t>(this);
        QFuture<system_call_wrapper_error_t>  res =
            QtConcurrent::run(CSystemCallWrapper::run_vagrant_up_in_terminal, directory);
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

#endif
