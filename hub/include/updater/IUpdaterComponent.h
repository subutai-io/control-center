#ifndef IUPDATERCOMPONENT_H
#define IUPDATERCOMPONENT_H

#include <QDebug>
#include <QObject>
#include <QString>
#include <atomic>
#include "DlgNotification.h"

namespace update_system {
typedef enum hub_component_updater_error {
  CHUE_SUCCESS = 0,
  CHUE_IN_PROGRESS,
  CHUE_NOT_ON_KURJUN,
  CHUE_FAILED
} hub_component_updater_error_t,
    chue_t;
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The atomic_locker struct uses as wrapper on std::atomic<bool>* ab
 * It sets ab in constructor and clears ab in destructor
 */
struct atomic_locker {
  std::atomic<bool>* m_ab;
  atomic_locker(std::atomic<bool>* ab) : m_ab(ab) { *m_ab = true; }
  ~atomic_locker() { *m_ab = false; }
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief The IUpdaterComponent interface for all ipdatable components
 */
class IUpdaterComponent : public QObject {
  Q_OBJECT
 private:
 protected:
  QString m_component_id;
  std::atomic<bool> m_in_progress;

  virtual bool update_available_internal() = 0;
  virtual chue_t update_internal() = 0;
  virtual void update_post_action(bool success) = 0;
  virtual chue_t install_internal() = 0;
  virtual void install_post_interntal(bool success) = 0;
  virtual chue_t uninstall_internal() = 0;
  virtual void uninstall_post_internal(bool success) = 0;

 public:
  static const QString P2P;
  static const QString TRAY;
  static const QString X2GO;
  static const QString VAGRANT;
  static const QString ORACLE_VIRTUALBOX;
  static const QString CHROME;
  static const QString E2E;
  static const QString VAGRANT_SUBUTAI;
  static const QString VAGRANT_VBGUEST;
  static const QString SUBUTAI_BOX;

  bool is_in_progress() { return m_in_progress; }
  std::pair<quint64, quint64> last_pb_value;
  const std::pair<quint64, quint64>& get_last_pb_value() {
    return last_pb_value;
  }

  IUpdaterComponent() : m_in_progress(false) {}
  virtual ~IUpdaterComponent() {}

  static const QString& component_id_to_user_view(const QString& id);
  static const QString& component_id_changelog(const QString& id);
  static DlgNotification::NOTIFICATION_ACTION_TYPE
  component_id_to_notification_action(const QString& id);

  bool update_available() {
    bool res = update_available_internal();
    if (res) {
      emit update_available_changed(m_component_id);
    }
    return res;
  }

  chue_t update() {
    if (m_in_progress) return CHUE_IN_PROGRESS;
    m_in_progress = true;
    return update_internal();
  }

  chue_t install() {
    if (m_in_progress) return CHUE_IN_PROGRESS;
    m_in_progress = true;
    return install_internal();
  }

  chue_t uninstall() {
    if (m_in_progress) return CHUE_IN_PROGRESS;
    m_in_progress = true;
    return uninstall_internal();
  }

  const QString& component_id() const { return m_component_id; }

 protected slots:
  void update_finished_sl(bool success) {
    update_post_action(success);
    m_in_progress = false;
    emit update_finished(m_component_id, success);
  }

  void install_finished_sl(bool success) {
    install_post_interntal(success);
    m_in_progress = false;
    emit install_finished(m_component_id, success);
  }

  void uninstall_finished_sl(bool success) {
    uninstall_post_internal(success);
    m_in_progress = false;
    emit uninstall_finished(m_component_id, success);
  }

  void update_progress_sl(qint64 part, qint64 total) {
    last_pb_value.first = part;
    last_pb_value.second = total;
    emit update_progress(m_component_id, part, total);
  }

 signals:
  void update_progress(const QString& component_id, qint64 part, qint64 total);
  void update_finished(const QString& component_id, bool success);
  void install_finished(const QString& component_id, bool success);
  void uninstall_finished(const QString& component_id, bool success);
  void update_available_changed(const QString& component_id);
};

}  // namespace update_system

#endif  // IUPDATERCOMPONENT_H
