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

class PeerController : public QObject {
  Q_OBJECT

private:
  PeerController(QObject* parent = nullptr);
  virtual ~PeerController();

  std::vector<CLocalPeer> m_local_peers;

  QTimer m_refresh_timer;

  int number_threads; // to emit signal when all finished

  void search_local();
  void get_peer_info(const QFileInfo &fi, QDir dir);
  QString parse_name(const QString &name);

public:

  static const int REFRESH_DELAY_SEC = 8;
  static PeerController* Instance() {
    static PeerController inst;
    return &inst;
  }

  void init();
  void refresh();
  void output_from_fingerprint(const QString &name, const QString &dir, const QStringList &output);

  const std::vector<CLocalPeer>& local_peers() const {
      return m_local_peers;
  }

private slots:
  void refresh_timer_timeout();
signals:
  void local_peer_list_updated();
};

class GetFingerPrint : public QObject{
    Q_OBJECT
    QString directory;
public:
    GetFingerPrint(QObject *parent = nullptr) : QObject(parent){}

    void init (const QString &directory){
        this->directory = directory;
    }

    void startWork() {
        QThread* thread = new QThread();
        connect(thread, &QThread::started,
                this, &GetFingerPrint::execute_remote_command);
        connect(this, &GetFingerPrint::outputReceived,
                thread, &QThread::quit);
        connect(thread, &QThread::finished,
                this, &GetFingerPrint::deleteLater);
        connect(thread, &QThread::finished,
                thread, &QThread::deleteLater);
        this->moveToThread(thread);
        thread->start();
    }

    void execute_remote_command() {
        QFutureWatcher<QStringList> *watcher
            = new QFutureWatcher<QStringList>(this);
        QFuture<QStringList>  res =
            QtConcurrent::run(CSystemCallWrapper::vagrant_fingerprint, directory);
        watcher->setFuture(res);
        connect(watcher, &QFutureWatcher<QStringList>::finished, [this, res](){
          emit this->outputReceived(res.result());
        });
    }
signals:
    void outputReceived(QStringList res);
};


#endif
