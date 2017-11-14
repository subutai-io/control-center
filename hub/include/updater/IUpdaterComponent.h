#ifndef IUPDATERCOMPONENT_H
#define IUPDATERCOMPONENT_H

#include <QString>
#include <atomic>
#include <QObject>
#include <QDebug>
#include "DlgNotification.h"

namespace update_system {
  typedef enum hub_component_updater_error {
    CHUE_SUCCESS = 0,
    CHUE_IN_PROGRESS,
    CHUE_NOT_ON_KURJUN,
    CHUE_FAILED
  } hub_component_updater_error_t, chue_t;
  ////////////////////////////////////////////////////////////////////////////

  struct atomic_locker {
    std::atomic<bool>* m_ab;
    atomic_locker(std::atomic<bool>* ab) :
      m_ab(ab) {
      *m_ab = true;
    }
    ~atomic_locker() { *m_ab = false;}
  };
  ////////////////////////////////////////////////////////////////////////////

  class IUpdaterComponent : public QObject {
    Q_OBJECT
  private:
  protected:
    QString m_component_id;
    std::atomic<bool> m_in_progress;

    virtual bool update_available_internal() = 0;
    virtual chue_t update_internal() = 0;
    virtual void update_post_action(bool success) = 0;

  public:

    static const QString P2P;
    static const QString TRAY;
    static const QString RH;
    static const QString RHMANAGEMENT;

    IUpdaterComponent() : m_in_progress(false){}
    virtual ~IUpdaterComponent(){}

    static const QString& component_id_to_user_view(const QString &id);
    static DlgNotification::NOTIFICATION_ACTION_TYPE component_id_to_notification_action(const QString& id);


    bool update_available() {
      bool res = update_available_internal();
      if (res) {
        emit update_available_changed(m_component_id);
      }
      return res;
    }

    chue_t update() {
      if (m_in_progress) return CHUE_IN_PROGRESS;
      atomic_locker al(&m_in_progress);
      return update_internal();
    }

    const QString& component_id() const {return m_component_id;}

  protected slots:
    void update_finished_sl(bool success) {
      update_post_action(success);
      emit update_finished(m_component_id, success);
    }

    void update_progress_sl(qint64 part, qint64 total){
      emit update_progress(m_component_id, part, total);
    }

  signals:
    void update_progress(const QString& component_id, qint64 part, qint64 total);
    void update_finished(const QString& component_id, bool success);
    void update_available_changed(const QString& component_id);
  };

}

#endif // IUPDATERCOMPONENT_H
