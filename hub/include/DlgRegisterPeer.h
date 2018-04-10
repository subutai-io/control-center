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
    void setLocalAddr(const QString local_ip){
        ip_addr = local_ip;
    }
    void setRegistrationMode();
    void setUnregistrationMode();

private:
    Ui::DlgRegisterPeer *ui;
    QString ip_addr;

private slots:
    void registerPeer();
    void unregisterPeer();
};

#endif // DLGREGISTERPEER_H
