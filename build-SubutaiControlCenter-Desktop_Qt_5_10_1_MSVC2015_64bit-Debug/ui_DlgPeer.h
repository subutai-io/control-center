/********************************************************************************
** Form generated from reading UI file 'DlgPeer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGPEER_H
#define UI_DLGPEER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgPeer
{
public:
    QGridLayout *gridLayout;
    QGroupBox *gr_ssh;
    QGridLayout *gridLayout_2;
    QLineEdit *le_ip;
    QLineEdit *le_pass;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *le_port;
    QPushButton *btn_ssh_peer;
    QLabel *label_4;
    QLineEdit *le_user;
    QLabel *label;
    QLabel *lbl_env;
    QLabel *lbl_env_status;
    QFrame *line_3;
    QGroupBox *gr_peer_control;
    QGridLayout *gridLayout_3;
    QPushButton *btn_destroy;
    QLabel *le_status;
    QLineEdit *le_ram;
    QVBoxLayout *verticalLayout_3;
    QPushButton *btn_start;
    QPushButton *btn_stop;
    QLabel *lbl_name;
    QPushButton *btn_reload;
    QComboBox *cmb_bridge;
    QVBoxLayout *verticalLayout_4;
    QPushButton *btn_register;
    QPushButton *btn_unregister;
    QLabel *lbl_ram;
    QLineEdit *le_disk;
    QLineEdit *le_cpu;
    QCheckBox *change_confugre;
    QLineEdit *le_name;
    QLabel *lbl_cpu;
    QLabel *lbl_status;
    QLabel *lbl_bridge;
    QLabel *lbl_disk;
    QFrame *line_2;
    QVBoxLayout *env_status;
    QVBoxLayout *env_owner;
    QCheckBox *show_peer_control;
    QVBoxLayout *env_name;
    QPushButton *btn_launch_console;
    QPushButton *btn_peer_on_hub;
    QCheckBox *show_ssh;
    QLabel *lbl_env_owner;
    QFrame *line_1;

    void setupUi(QDialog *DlgPeer)
    {
        if (DlgPeer->objectName().isEmpty())
            DlgPeer->setObjectName(QStringLiteral("DlgPeer"));
        DlgPeer->resize(636, 510);
        DlgPeer->setLayoutDirection(Qt::LeftToRight);
        gridLayout = new QGridLayout(DlgPeer);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gr_ssh = new QGroupBox(DlgPeer);
        gr_ssh->setObjectName(QStringLiteral("gr_ssh"));
        gridLayout_2 = new QGridLayout(gr_ssh);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        le_ip = new QLineEdit(gr_ssh);
        le_ip->setObjectName(QStringLiteral("le_ip"));

        gridLayout_2->addWidget(le_ip, 0, 1, 1, 1);

        le_pass = new QLineEdit(gr_ssh);
        le_pass->setObjectName(QStringLiteral("le_pass"));

        gridLayout_2->addWidget(le_pass, 1, 3, 1, 1);

        label_2 = new QLabel(gr_ssh);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(gr_ssh);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout_2->addWidget(label_3, 0, 2, 1, 1);

        le_port = new QLineEdit(gr_ssh);
        le_port->setObjectName(QStringLiteral("le_port"));

        gridLayout_2->addWidget(le_port, 1, 1, 1, 1);

        btn_ssh_peer = new QPushButton(gr_ssh);
        btn_ssh_peer->setObjectName(QStringLiteral("btn_ssh_peer"));

        gridLayout_2->addWidget(btn_ssh_peer, 2, 3, 1, 1);

        label_4 = new QLabel(gr_ssh);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout_2->addWidget(label_4, 1, 2, 1, 1);

        le_user = new QLineEdit(gr_ssh);
        le_user->setObjectName(QStringLiteral("le_user"));

        gridLayout_2->addWidget(le_user, 0, 3, 1, 1);

        label = new QLabel(gr_ssh);
        label->setObjectName(QStringLiteral("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 1);


        gridLayout->addWidget(gr_ssh, 8, 1, 1, 3);

        lbl_env = new QLabel(DlgPeer);
        lbl_env->setObjectName(QStringLiteral("lbl_env"));
        lbl_env->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbl_env, 1, 1, 1, 1);

        lbl_env_status = new QLabel(DlgPeer);
        lbl_env_status->setObjectName(QStringLiteral("lbl_env_status"));
        lbl_env_status->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbl_env_status, 1, 3, 1, 1);

        line_3 = new QFrame(DlgPeer);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_3, 2, 3, 1, 1);

        gr_peer_control = new QGroupBox(DlgPeer);
        gr_peer_control->setObjectName(QStringLiteral("gr_peer_control"));
        gridLayout_3 = new QGridLayout(gr_peer_control);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        btn_destroy = new QPushButton(gr_peer_control);
        btn_destroy->setObjectName(QStringLiteral("btn_destroy"));

        gridLayout_3->addWidget(btn_destroy, 6, 1, 1, 1);

        le_status = new QLabel(gr_peer_control);
        le_status->setObjectName(QStringLiteral("le_status"));

        gridLayout_3->addWidget(le_status, 1, 3, 1, 1);

        le_ram = new QLineEdit(gr_peer_control);
        le_ram->setObjectName(QStringLiteral("le_ram"));

        gridLayout_3->addWidget(le_ram, 2, 3, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        btn_start = new QPushButton(gr_peer_control);
        btn_start->setObjectName(QStringLiteral("btn_start"));

        verticalLayout_3->addWidget(btn_start);

        btn_stop = new QPushButton(gr_peer_control);
        btn_stop->setObjectName(QStringLiteral("btn_stop"));

        verticalLayout_3->addWidget(btn_stop);


        gridLayout_3->addLayout(verticalLayout_3, 5, 1, 1, 1);

        lbl_name = new QLabel(gr_peer_control);
        lbl_name->setObjectName(QStringLiteral("lbl_name"));

        gridLayout_3->addWidget(lbl_name, 1, 0, 1, 1);

        btn_reload = new QPushButton(gr_peer_control);
        btn_reload->setObjectName(QStringLiteral("btn_reload"));

        gridLayout_3->addWidget(btn_reload, 6, 3, 1, 1);

        cmb_bridge = new QComboBox(gr_peer_control);
        cmb_bridge->setObjectName(QStringLiteral("cmb_bridge"));

        gridLayout_3->addWidget(cmb_bridge, 3, 3, 1, 1);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        btn_register = new QPushButton(gr_peer_control);
        btn_register->setObjectName(QStringLiteral("btn_register"));

        verticalLayout_4->addWidget(btn_register);

        btn_unregister = new QPushButton(gr_peer_control);
        btn_unregister->setObjectName(QStringLiteral("btn_unregister"));

        verticalLayout_4->addWidget(btn_unregister);


        gridLayout_3->addLayout(verticalLayout_4, 5, 3, 1, 1);

        lbl_ram = new QLabel(gr_peer_control);
        lbl_ram->setObjectName(QStringLiteral("lbl_ram"));

        gridLayout_3->addWidget(lbl_ram, 2, 2, 1, 1);

        le_disk = new QLineEdit(gr_peer_control);
        le_disk->setObjectName(QStringLiteral("le_disk"));

        gridLayout_3->addWidget(le_disk, 2, 1, 1, 1);

        le_cpu = new QLineEdit(gr_peer_control);
        le_cpu->setObjectName(QStringLiteral("le_cpu"));

        gridLayout_3->addWidget(le_cpu, 3, 1, 1, 1);

        change_confugre = new QCheckBox(gr_peer_control);
        change_confugre->setObjectName(QStringLiteral("change_confugre"));

        gridLayout_3->addWidget(change_confugre, 4, 1, 1, 1);

        le_name = new QLineEdit(gr_peer_control);
        le_name->setObjectName(QStringLiteral("le_name"));

        gridLayout_3->addWidget(le_name, 1, 1, 1, 1);

        lbl_cpu = new QLabel(gr_peer_control);
        lbl_cpu->setObjectName(QStringLiteral("lbl_cpu"));

        gridLayout_3->addWidget(lbl_cpu, 3, 0, 1, 1);

        lbl_status = new QLabel(gr_peer_control);
        lbl_status->setObjectName(QStringLiteral("lbl_status"));
        lbl_status->setMinimumSize(QSize(44, 0));

        gridLayout_3->addWidget(lbl_status, 1, 2, 1, 1);

        lbl_bridge = new QLabel(gr_peer_control);
        lbl_bridge->setObjectName(QStringLiteral("lbl_bridge"));

        gridLayout_3->addWidget(lbl_bridge, 3, 2, 1, 1);

        lbl_disk = new QLabel(gr_peer_control);
        lbl_disk->setObjectName(QStringLiteral("lbl_disk"));

        gridLayout_3->addWidget(lbl_disk, 2, 0, 1, 1);


        gridLayout->addWidget(gr_peer_control, 10, 1, 1, 3);

        line_2 = new QFrame(DlgPeer);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 2, 2, 1, 1);

        env_status = new QVBoxLayout();
        env_status->setObjectName(QStringLiteral("env_status"));

        gridLayout->addLayout(env_status, 3, 3, 1, 1);

        env_owner = new QVBoxLayout();
        env_owner->setObjectName(QStringLiteral("env_owner"));

        gridLayout->addLayout(env_owner, 3, 2, 1, 1);

        show_peer_control = new QCheckBox(DlgPeer);
        show_peer_control->setObjectName(QStringLiteral("show_peer_control"));

        gridLayout->addWidget(show_peer_control, 9, 2, 1, 1, Qt::AlignHCenter);

        env_name = new QVBoxLayout();
        env_name->setObjectName(QStringLiteral("env_name"));

        gridLayout->addLayout(env_name, 3, 1, 1, 1);

        btn_launch_console = new QPushButton(DlgPeer);
        btn_launch_console->setObjectName(QStringLiteral("btn_launch_console"));

        gridLayout->addWidget(btn_launch_console, 6, 3, 1, 1);

        btn_peer_on_hub = new QPushButton(DlgPeer);
        btn_peer_on_hub->setObjectName(QStringLiteral("btn_peer_on_hub"));

        gridLayout->addWidget(btn_peer_on_hub, 6, 1, 1, 1);

        show_ssh = new QCheckBox(DlgPeer);
        show_ssh->setObjectName(QStringLiteral("show_ssh"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(show_ssh->sizePolicy().hasHeightForWidth());
        show_ssh->setSizePolicy(sizePolicy);
        show_ssh->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(show_ssh, 6, 2, 1, 1, Qt::AlignHCenter);

        lbl_env_owner = new QLabel(DlgPeer);
        lbl_env_owner->setObjectName(QStringLiteral("lbl_env_owner"));
        lbl_env_owner->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(lbl_env_owner, 1, 2, 1, 1);

        line_1 = new QFrame(DlgPeer);
        line_1->setObjectName(QStringLiteral("line_1"));
        line_1->setFrameShape(QFrame::HLine);
        line_1->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_1, 2, 1, 1, 1);


        retranslateUi(DlgPeer);

        QMetaObject::connectSlotsByName(DlgPeer);
    } // setupUi

    void retranslateUi(QDialog *DlgPeer)
    {
        DlgPeer->setWindowTitle(QApplication::translate("DlgPeer", "Dialog", nullptr));
        gr_ssh->setTitle(QString());
        label_2->setText(QApplication::translate("DlgPeer", "SSH Port", nullptr));
        label_3->setText(QApplication::translate("DlgPeer", "User", nullptr));
#ifndef QT_NO_TOOLTIP
        btn_ssh_peer->setToolTip(QApplication::translate("DlgPeer", "<html><head/><body><p>SSH into RH</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        btn_ssh_peer->setText(QApplication::translate("DlgPeer", "SSH into Peer", nullptr));
        label_4->setText(QApplication::translate("DlgPeer", "Pass", nullptr));
        label->setText(QApplication::translate("DlgPeer", "IP", nullptr));
        lbl_env->setText(QApplication::translate("DlgPeer", "Environment", nullptr));
        lbl_env_status->setText(QApplication::translate("DlgPeer", "Status", nullptr));
        gr_peer_control->setTitle(QString());
        btn_destroy->setText(QApplication::translate("DlgPeer", "Destroy", nullptr));
        le_status->setText(QApplication::translate("DlgPeer", "Peer is running", nullptr));
        btn_start->setText(QApplication::translate("DlgPeer", "Start", nullptr));
        btn_stop->setText(QApplication::translate("DlgPeer", "Stop", nullptr));
        lbl_name->setText(QApplication::translate("DlgPeer", "Name", nullptr));
        btn_reload->setText(QApplication::translate("DlgPeer", "Reload", nullptr));
        btn_register->setText(QApplication::translate("DlgPeer", "Register to Bazaar", nullptr));
        btn_unregister->setText(QApplication::translate("DlgPeer", "Unregister from Bazaar", nullptr));
        lbl_ram->setText(QApplication::translate("DlgPeer", "RAM", nullptr));
        change_confugre->setText(QApplication::translate("DlgPeer", "Locked configs", nullptr));
        lbl_cpu->setText(QApplication::translate("DlgPeer", "CPU", nullptr));
        lbl_status->setText(QApplication::translate("DlgPeer", "State", nullptr));
        lbl_bridge->setText(QApplication::translate("DlgPeer", "Bridge", nullptr));
        lbl_disk->setText(QApplication::translate("DlgPeer", "Disk", nullptr));
        show_peer_control->setText(QApplication::translate("DlgPeer", "Advanced settings", nullptr));
        btn_launch_console->setText(QApplication::translate("DlgPeer", "Launch  Subutai Console", nullptr));
        btn_peer_on_hub->setText(QApplication::translate("DlgPeer", "Peer on Bazaar", nullptr));
        show_ssh->setText(QApplication::translate("DlgPeer", "Show SSH box", nullptr));
        lbl_env_owner->setText(QApplication::translate("DlgPeer", "Owner", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgPeer: public Ui_DlgPeer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGPEER_H
