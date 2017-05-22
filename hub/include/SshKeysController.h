#ifndef SSHKEYSCONTROLLER_H
#define SSHKEYSCONTROLLER_H

#include <QObject>

class CSshKeysController : public QObject {
  Q_OBJECT
private:
  QStringList m_lst_key_files;
  QStringList m_lst_key_content;

  QString m_current_key;
  int32_t m_current_key_col;

  std::vector<bool> m_lst_all_selected;
  typedef std::vector<std::vector<bool> > bit_matrix;
  bit_matrix m_original_bit_matrix;
  bit_matrix m_current_bit_matrix;

  CSshKeysController();
  virtual ~CSshKeysController();

public:
  void refresh_key_files();
  void rebuild_bit_matrix();
  void generate_new_ssh_key(QWidget *parent);
  void send_data_to_hub();

  static CSshKeysController& Instance() {
    static CSshKeysController instance;
    return instance;
  }

  bool has_current_key() const {return !m_current_key.isEmpty();}
  bool something_changed() const {return m_original_bit_matrix != m_current_bit_matrix;}
  void set_current_key(const QString& key) ;

  void set_key_environments_bit(int index, bool bit);
  bool get_key_environments_bit(int index) const;

  bool current_key_is_allselected() const;
  bool set_current_key_allselected(bool flag);

  const QStringList& lst_key_files() const {return m_lst_key_files;}
  QStringList keys_in_environment(const QString& env_id) const;

private slots:
  void ssh_key_send_progress_sl(int part, int total);
  void environments_updated(int rr);

signals:
  void key_files_changed();
  void ssh_key_send_progress(int part, int total);
};

#endif // SSHKEYSCONTROLLER_H
