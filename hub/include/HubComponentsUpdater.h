#ifndef HUBCOMPONENTSUPDATER_H
#define HUBCOMPONENTSUPDATER_H

#include <QObject>
#include <atomic>

typedef enum hub_component_updater_error {
  CHUE_SUCCESS = 0,
  CHUE_IN_PROGRESS,
  CHUE_NOT_ON_KURJUN,
  CHUE_FAILED
} hub_component_updater_error_t, chue_t;

class CHubComponentsUpdater : public QObject {
  Q_OBJECT
private:
  struct atomic_locker {
    std::atomic<bool>* m_ab;
    atomic_locker(std::atomic<bool>* ab) :
      m_ab(ab) {
      *m_ab = true;
    }
    ~atomic_locker() { *m_ab = false;}
    bool locked() const {return *m_ab;}
  };

  CHubComponentsUpdater();
  ~CHubComponentsUpdater();

  std::atomic<int> m_tray_update_checks;
  std::atomic<bool> m_tray_updating;
  std::atomic<bool> m_subutai_rh_updating;

  std::atomic<int> m_p2p_update_checks;
  std::atomic<bool> m_p2p_updating;


  std::string p2p_path(void) const;
public:

  static CHubComponentsUpdater* Instance() {
    static CHubComponentsUpdater instance;
    return &instance;
  }

  bool tray_check_for_update(void);
  chue_t tray_update(void);

  chue_t subutai_rh_update(void);

  bool p2p_check_for_update(void);
  chue_t p2p_update(void);

private slots:
  void download_file_progress_sl(qint64 cur, qint64 full);

  void tray_downloading_finished();
  void p2p_downloading_finished();

signals:
  void download_file_progress(qint64, qint64);
};

#endif // HUBCOMPONENTSUPDATER_H
