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

private:
    Ui::DlgRegisterPeer *ui;
    QString ip_addr;
    QString peer_name;

private slots:
    void registerPeer();
    void unregisterPeer();
};

#endif // DLGREGISTERPEER_H
