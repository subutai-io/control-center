#ifndef IUPDATERCOMPONENT_H
#define IUPDATERCOMPONENT_H

#include <QString>
#include <atomic>
#include <QObject>

namespace update_system {
  typedef enum hub_component_updater_error {
    CHUE_SUCCESS = 0,
    CHUE_IN_PROGRESS,
    CHUE_NOT_ON_KURJUN,
    CHUE_FAILED
  } hub_component_updater_error_t, chue_t;
  ////////////////////////////////////////////////////////////////////////////

  /*!
   * \brief The atomic_locker is RAII struct. Constructor sets some boolean variable to true. Destructor - to false
   */
  struct atomic_locker {
    std::atomic<bool>* m_ab;
    atomic_locker(std::atomic<bool>* ab) :
      m_ab(ab) {
      *m_ab = true;
    }
    ~atomic_locker() { *m_ab = false;}
  };
  ////////////////////////////////////////////////////////////////////////////

  /*!
   * \brief The IUpdaterComponent is abstract class with pure abstract methods.
   * It provides interface for updating system. Detailed implementation is placed in derived classes.
   */
  class IUpdaterComponent : public QObject {
    Q_OBJECT
  private:
  protected:
    QString m_component_id;
    std::atomic<bool> m_in_progress;

    /*!
     * \brief Is update available
     */
    virtual bool update_available_internal() = 0;

    /*!
     * \brief Update component
     */
    virtual chue_t update_internal() = 0;

    /*!
     * \brief Run post update action
     */
    virtual void update_post_action(bool success) = 0;

  public:

    static const QString P2P;
    static const QString TRAY;
    static const QString RH;
    static const QString RHMANAGEMENT;

    IUpdaterComponent() : m_in_progress(false){}
    virtual ~IUpdaterComponent(){}

    static const QString& component_id_to_user_view(const QString &id);

    /*!
     * \brief If update is available returns true and emits `update_available_changed` signal
     */
    bool update_available() {
      bool res = update_available_internal();
      if (res) {
        emit update_available_changed(m_component_id);
      }
      return res;
    }

    /*!
     * \brief Check if update in progress. If not - try to update by calling virtual method. So that method should be implemented
     * in derived class
     */
    chue_t update() {
      if (m_in_progress) return CHUE_IN_PROGRESS;
      atomic_locker al(&m_in_progress);
      return update_internal();
    }

    /*!
     * \brief Component identificator uses in many places. Be careful with this field
     * \return
     */
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
