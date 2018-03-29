#ifndef HUBCOMPONENTSUPDATER_H
#define HUBCOMPONENTSUPDATER_H

#include <QObject>
#include <QTimer>
#include <map>
#include <QFuture>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

#include "updater/IUpdaterComponent.h"
#include "NotificationObserver.h"
#include "DlgNotification.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"

namespace update_system {

  /**
   * @brief Wraps common characteristics for Subutai components like autoupdate, update frequency etc.
   */
  class CUpdaterComponentItem : public QObject {
    Q_OBJECT
  private:
    QTimer m_timer;
    IUpdaterComponent* m_component;

  public:
    //PUBLIC FIELD!!! ACHTUNG!!! :-D
    bool autoupdate;

    CUpdaterComponentItem() : m_component(NULL), autoupdate(false){
      connect(&m_timer, &QTimer::timeout, this, &CUpdaterComponentItem::timer_timeout_sl);
    }

    explicit CUpdaterComponentItem(IUpdaterComponent* component) :
      m_component(component), autoupdate(false) {
      connect(&m_timer, &QTimer::timeout, this, &CUpdaterComponentItem::timer_timeout_sl);
    }

    CUpdaterComponentItem(const CUpdaterComponentItem& arg); //copy constructor prohibited

    ~CUpdaterComponentItem(){}

    CUpdaterComponentItem& operator=(const CUpdaterComponentItem& rh) {
      this->m_component = rh.m_component;
      return *this;
    }

    void set_timer_interval(int msec) {
      m_timer.setInterval(msec);
    }

    void timer_start() {m_timer.start();}
    void timer_stop() {m_timer.stop();}

    IUpdaterComponent* Component() const {return m_component;}

  private slots:
    void timer_timeout_sl() {
      QString cid = m_component ? m_component->component_id() : "component_eq_null";
      emit timer_timeout(cid);
    }
  signals:
    void timer_timeout(const QString& component_id);
  };
  ////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Main update system class. Controls p2p, RH, tray and RH Management components.
   */
  class CHubComponentsUpdater : public QObject {
    Q_OBJECT
  private:

    CHubComponentsUpdater();
    ~CHubComponentsUpdater();

    std::map<QString, CUpdaterComponentItem> m_dct_components;
    void set_update_freq(const QString& component_id, CSettingsManager::update_freq_t freq);
    void set_component_autoupdate(const QString& component_id,
                                  bool autoupdate);

  public:

    static CHubComponentsUpdater* Instance() {
      static CHubComponentsUpdater instance;
      return &instance;
    }

    void set_p2p_update_freq();
    void set_rh_update_freq();
    void set_tray_update_freq();
    void set_rh_management_update_freq();
    void set_p2p_autoupdate();
    void set_rh_autoupdate();
    void set_tray_autoupdate();
    void set_rh_management_autoupdate();

    /**
     * @brief Checks if update for "component_id" is available
     * @return true if available, false otherwise
     */
    bool is_update_available(const QString& component_id);

    /**
     * @brief Update component "component_id" immediately
     */
    void force_update(const QString& component_id);

    void force_update_p2p();
    void force_update_tray();
    void force_update_rh();
    void force_update_rhm();

    /**
      * @brief Instal implementation for components
      *
      */
    void install(const QString& component_id);
    void install_p2p();
    void install_x2go();
    QString component_name(const QString& component_id);

  private slots:

    void update_component_timer_timeout(const QString& component_id);
    void update_component_progress_sl(const QString &file_id, qint64 cur, qint64 full);
    void update_component_finished_sl(const QString &file_id, bool replaced);
    void install_component_finished_sl(const QString &file_id, bool replaced);

  signals:
    void download_file_progress(const QString& file_id, qint64 rec, qint64 total);
    void updating_finished(const QString& file_id, bool success);
    void update_available(const QString& file_id);
    void installing_finished(const QString & file_id, bool success);
    void install_component_started(const QString &file_id);
  };
}

// USED FOR P2P INSTALLATION
class SilentPackageInstallerP2P : public QObject{
  Q_OBJECT
  QString dir, file_name;

public:
  SilentPackageInstallerP2P(QObject *parent = nullptr) : QObject (parent){}
  void init (const QString &dir,
             const QString &file_name){
      this->dir = dir;
      this->file_name = file_name;
  }

  void startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentPackageInstallerP2P::execute_remote_command);
    connect(this, &SilentPackageInstallerP2P::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentPackageInstallerP2P::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }


  void execute_remote_command() {
    //QStringList output;
    QFutureWatcher<system_call_wrapper_error_t> *watcher
        = new QFutureWatcher<system_call_wrapper_error_t>(this);

    QFuture<system_call_wrapper_error_t>  res =
        QtConcurrent::run(CSystemCallWrapper::install_p2p, dir, file_name);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
      emit this->outputReceived(res.result() == SCWE_SUCCESS);
    });
  }

signals:
  void outputReceived(bool success);
};
// FOR X2GO INSTALLATION
class SilentPackageInstallerX2GO : public QObject{
  Q_OBJECT
  QString dir, file_name;

public:
  SilentPackageInstallerX2GO(QObject *parent = nullptr) : QObject (parent){}
  void init (const QString &dir,
             const QString &file_name){
      this->dir = dir;
      this->file_name = file_name;
  }

  void startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentPackageInstallerX2GO::execute_remote_command);
    connect(this, &SilentPackageInstallerX2GO::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentPackageInstallerX2GO::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }


  void execute_remote_command() {
    //QStringList output;
    QFutureWatcher<system_call_wrapper_error_t> *watcher
        = new QFutureWatcher<system_call_wrapper_error_t>(this);

    QFuture<system_call_wrapper_error_t>  res =
        QtConcurrent::run(CSystemCallWrapper::install_x2go, dir, file_name);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
      emit this->outputReceived(res.result() == SCWE_SUCCESS);
    });
  }

signals:
  void outputReceived(bool success);
};
// FOR VAGRANT INSTALLATION
class SilentPackageInstallerVAGRANT : public QObject{
  Q_OBJECT
  QString dir, file_name;

public:
  SilentPackageInstallerVAGRANT(QObject *parent = nullptr) : QObject (parent){}
  void init (const QString &dir,
             const QString &file_name){
      this->dir = dir;
      this->file_name = file_name;
  }

  void startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentPackageInstallerVAGRANT::execute_remote_command);
    connect(this, &SilentPackageInstallerVAGRANT::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentPackageInstallerVAGRANT::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }


  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher
        = new QFutureWatcher<system_call_wrapper_error_t>(this);

    QFuture<system_call_wrapper_error_t>  res =
        QtConcurrent::run(CSystemCallWrapper::install_vagrant, dir, file_name);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
      emit this->outputReceived(res.result() == SCWE_SUCCESS);
    });
  }

signals:
  void outputReceived(bool success);
};
//FOR VIRTUALBOX INSTALLATION
class SilentPackageInstallerORACLE_VIRTUALBOX : public QObject{
  Q_OBJECT
  QString dir, file_name;

public:
  SilentPackageInstallerORACLE_VIRTUALBOX(QObject *parent = nullptr) : QObject (parent){}
  void init (const QString &dir,
             const QString &file_name){
      this->dir = dir;
      this->file_name = file_name;
  }

  void startWork() {
    QThread* thread = new QThread();
    connect(thread, &QThread::started,
            this, &SilentPackageInstallerORACLE_VIRTUALBOX::execute_remote_command);
    connect(this, &SilentPackageInstallerORACLE_VIRTUALBOX::outputReceived,
            thread, &QThread::quit);
    connect(thread, &QThread::finished,
            this, &SilentPackageInstallerORACLE_VIRTUALBOX::deleteLater);
    connect(thread, &QThread::finished,
            thread, &QThread::deleteLater);
    this->moveToThread(thread);
    thread->start();
  }


  void execute_remote_command() {
    QFutureWatcher<system_call_wrapper_error_t> *watcher
        = new QFutureWatcher<system_call_wrapper_error_t>(this);

    QFuture<system_call_wrapper_error_t>  res =
        QtConcurrent::run(CSystemCallWrapper::install_oracle_virtualbox, dir, file_name);
    watcher->setFuture(res);
    connect(watcher, &QFutureWatcher<system_call_wrapper_error_t>::finished, [this, res](){
      emit this->outputReceived(res.result() == SCWE_SUCCESS);
    });
  }

signals:
  void outputReceived(bool success);
};
#endif // HUBCOMPONENTSUPDATER_H
