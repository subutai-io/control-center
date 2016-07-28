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

  struct file_atomic {
    //todo make it atomic
    std::atomic<int> checks;
    std::atomic<bool> in_progress;
    QString kurjun_file_name;

    file_atomic() : checks(0), in_progress(false), kurjun_file_name("") {}
    file_atomic(const char* kfn) :
      checks(0), in_progress(false), kurjun_file_name(kfn){}

    file_atomic(const file_atomic& arg) :
      checks(arg.checks.load()),
      in_progress(arg.in_progress.load()),
      kurjun_file_name(arg.kurjun_file_name) { }

    file_atomic& operator=(const file_atomic& arg) {
      checks = arg.checks.load();
      in_progress = arg.in_progress.load();
      kurjun_file_name = arg.kurjun_file_name;
      return *this;
    }
  };

  CHubComponentsUpdater();
  ~CHubComponentsUpdater();

  std::map<QString, file_atomic> m_dct_file_atomics;
  std::atomic<bool> m_subutai_rh_updating;

  std::string p2p_path(void) const;
  chue_t update_and_replace_file(const QString& file_id,
                               const QString& download_path,
                               const QString& file_to_replace_path);
public:

  static const QString STR_P2P;
  static const QString STR_TRAY;

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
  void download_file_progress_sl(QString file_id, qint64 cur, qint64 full);

  void file_downloading_finished(QString file_id);
  void file_replace_finished(QString file_id, bool replaced);

signals:

  void download_file_progress(QString file_id, qint64 rec, qint64 total);
  void updating_finished(QString file_id, bool success);

  void update_available(QString file_id);
};

#endif // HUBCOMPONENTSUPDATER_H
