#ifndef DLGREGISTERPEER_H
#define DLGREGISTERPEER_H

#include <QDialog>
namespace Ui {
class DlgRegisterPeer;
}

class DlgRegisterPeer : public QDialog
{
    Q_OBJECT

public:
    explicit DlgRegisterPeer(QWidget *parent = 0);
    ~DlgRegisterPeer();
    void init(const QString local_ip,
              const QString name);
    void setRegistrationMode();
    void setUnregistrationMode();
    QRegExp m_invalid_chars;
    bool m_password_state;
    static int dialog_used [10000];
    static int dialog_running [10000];
signals:
    void register_finished();
private:
    Ui::DlgRegisterPeer *ui;
    QString ip_addr;
    QString m_url_management;
    QString peer_name;
    QAction *m_show_password_action;

    bool check_errors(const int &err_code,
                      const int &http_code,
                      const int &network_error,
                      const QString& body);

private slots:
    void registerPeer();
    void unregisterPeer();
};

#endif // DLGREGISTERPEER_H
