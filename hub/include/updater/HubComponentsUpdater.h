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

enum cc_component{
    CC_P2P = 0,
    CC_X2GO,
    CC_VAGRANT,
    CC_VB,
    CC_CHROME,
    CC_FIREFOX,
    CC_E2E,
    CC_VAGRANT_SUBUTAI,
    CC_VAGRANT_VBGUEST,
    CC_SUBUTAI_BOX
};

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
      * @brief Install implementation for components
      *
      */
    void install(const QString& component_id);
    void uninstall(const QString& component_id);
    void install_p2p();
    void install_x2go();
    QString component_name(const QString& component_id);

    /* check for in progress each component and get progress value
    */
    bool is_in_progress(const QString& component_id);
    const std::pair <quint64, quint64>& get_last_pb_value(const QString& component_id);

  private slots:

    void update_component_timer_timeout(const QString& component_id);
    void update_component_progress_sl(const QString& component_id, qint64 cur, qint64 full);
    void update_component_finished_sl(const QString& component_id, bool replaced);
    void install_component_finished_sl(const QString& component_id, bool replaced);
    void uninstall_component_finished_sl(const QString& component_id, bool success);

  signals:
    void download_file_progress(const QString& component_id, qint64 rec, qint64 total);
    void updating_finished(const QString& component_id, bool success);
    void update_available(const QString& component_id);
    void installing_finished(const QString& component_id, bool success);
    void uninstalling_finished(const QString& component_id, bool success);
    void install_component_started(const QString& component_id);
    void uninstalling_component_finished(const QString& component_id, bool success);
    void uninstall_component_started(const QString& component_id);
  };
}

///////* class installs cc components in silent mode *///////////
class SilentInstaller : public QObject {
    Q_OBJECT
public:
    SilentInstaller(QObject *parent = nullptr) : QObject (parent) {}
    void init(const QString& dir, const QString &file_name, cc_component type);
    void startWork();
    void silentInstallation();

private:
    QString m_dir;
    QString m_file_name;
    cc_component m_type;

signals:
    void outputReceived(bool success);
};

/////* class updates cc components in silent mode *///////////////
class SilentUpdater : public QObject {
    Q_OBJECT
public:
    SilentUpdater(QObject *parent = nullptr) : QObject (parent) {}
    void init(const QString& dir, const QString &file_name, cc_component type);
    void startWork();
    void silentUpdate();

private:
    QString m_dir;
    QString m_file_name;
    cc_component m_type;

signals:
    void outputReceived(bool success);
};
/////* class deletes cc components in silent mode *///////////////
class SilentUninstaller : public QObject {
  Q_OBJECT
public:
  SilentUninstaller(QObject *parent = nullptr) : QObject(parent) {}
  void init(const QString& dir, const QString& file_name, cc_component type);
  void startWork();
  void silentUninstallation();

private:
  QString m_dir;
  QString m_file_name;
  cc_component m_type;

signals:
  void outputReceived(bool success);
};
#endif // HUBCOMPONENTSUPDATER_H
