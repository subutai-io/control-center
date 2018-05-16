/********************************************************************************
** Form generated from reading UI file 'DlgRegisterPeer.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGREGISTERPEER_H
#define UI_DLGREGISTERPEER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgRegisterPeer
{
public:
    QGridLayout *gridLayout;
    QPushButton *btn_cancel;
    QVBoxLayout *verticalLayout;
    QPushButton *btn_unregister;
    QPushButton *btn_register;
    QGridLayout *gridLayout_2;
    QLabel *lbl_password;
    QLineEdit *lne_peername;
    QLineEdit *lne_username;
    QLabel *lbl_peer_name;
    QLineEdit *lne_password;
    QLabel *lbl_username;
    QLabel *lbl_peer_scope;
    QComboBox *cmb_peer_scope;
    QLabel *lbl_info;

    void setupUi(QDialog *DlgRegisterPeer)
    {
        if (DlgRegisterPeer->objectName().isEmpty())
            DlgRegisterPeer->setObjectName(QStringLiteral("DlgRegisterPeer"));
        DlgRegisterPeer->resize(298, 184);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DlgRegisterPeer->sizePolicy().hasHeightForWidth());
        DlgRegisterPeer->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(DlgRegisterPeer);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        btn_cancel = new QPushButton(DlgRegisterPeer);
        btn_cancel->setObjectName(QStringLiteral("btn_cancel"));
        btn_cancel->setAutoDefault(false);

        gridLayout->addWidget(btn_cancel, 9, 2, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        btn_unregister = new QPushButton(DlgRegisterPeer);
        btn_unregister->setObjectName(QStringLiteral("btn_unregister"));

        verticalLayout->addWidget(btn_unregister);

        btn_register = new QPushButton(DlgRegisterPeer);
        btn_register->setObjectName(QStringLiteral("btn_register"));

        verticalLayout->addWidget(btn_register);


        gridLayout->addLayout(verticalLayout, 9, 1, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        lbl_password = new QLabel(DlgRegisterPeer);
        lbl_password->setObjectName(QStringLiteral("lbl_password"));

        gridLayout_2->addWidget(lbl_password, 1, 0, 1, 1);

        lne_peername = new QLineEdit(DlgRegisterPeer);
        lne_peername->setObjectName(QStringLiteral("lne_peername"));

        gridLayout_2->addWidget(lne_peername, 2, 1, 1, 1);

        lne_username = new QLineEdit(DlgRegisterPeer);
        lne_username->setObjectName(QStringLiteral("lne_username"));

        gridLayout_2->addWidget(lne_username, 0, 1, 1, 1);

        lbl_peer_name = new QLabel(DlgRegisterPeer);
        lbl_peer_name->setObjectName(QStringLiteral("lbl_peer_name"));

        gridLayout_2->addWidget(lbl_peer_name, 2, 0, 1, 1);

        lne_password = new QLineEdit(DlgRegisterPeer);
        lne_password->setObjectName(QStringLiteral("lne_password"));

        gridLayout_2->addWidget(lne_password, 1, 1, 1, 1);

        lbl_username = new QLabel(DlgRegisterPeer);
        lbl_username->setObjectName(QStringLiteral("lbl_username"));

        gridLayout_2->addWidget(lbl_username, 0, 0, 1, 1);

        lbl_peer_scope = new QLabel(DlgRegisterPeer);
        lbl_peer_scope->setObjectName(QStringLiteral("lbl_peer_scope"));

        gridLayout_2->addWidget(lbl_peer_scope, 3, 0, 1, 1);

        cmb_peer_scope = new QComboBox(DlgRegisterPeer);
        cmb_peer_scope->addItem(QString());
        cmb_peer_scope->addItem(QString());
        cmb_peer_scope->setObjectName(QStringLiteral("cmb_peer_scope"));

        gridLayout_2->addWidget(cmb_peer_scope, 3, 1, 1, 1);

        lbl_info = new QLabel(DlgRegisterPeer);
        lbl_info->setObjectName(QStringLiteral("lbl_info"));

        gridLayout_2->addWidget(lbl_info, 4, 0, 1, 2);


        gridLayout->addLayout(gridLayout_2, 5, 0, 1, 3);


        retranslateUi(DlgRegisterPeer);

        btn_unregister->setDefault(true);
        btn_register->setDefault(true);


        QMetaObject::connectSlotsByName(DlgRegisterPeer);
    } // setupUi

    void retranslateUi(QDialog *DlgRegisterPeer)
    {
        DlgRegisterPeer->setWindowTitle(QApplication::translate("DlgRegisterPeer", "Dialog", nullptr));
        btn_cancel->setText(QApplication::translate("DlgRegisterPeer", "Cancel", nullptr));
        btn_unregister->setText(QApplication::translate("DlgRegisterPeer", "Unregister", nullptr));
        btn_register->setText(QApplication::translate("DlgRegisterPeer", "Register", nullptr));
        lbl_password->setText(QApplication::translate("DlgRegisterPeer", "Console Password", nullptr));
        lbl_peer_name->setText(QApplication::translate("DlgRegisterPeer", "Peer name", nullptr));
        lbl_username->setText(QApplication::translate("DlgRegisterPeer", "Console Username", nullptr));
        lbl_peer_scope->setText(QApplication::translate("DlgRegisterPeer", "Peer Scope", nullptr));
        cmb_peer_scope->setItemText(0, QApplication::translate("DlgRegisterPeer", "Public", nullptr));
        cmb_peer_scope->setItemText(1, QApplication::translate("DlgRegisterPeer", "Private", nullptr));

        lbl_info->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class DlgRegisterPeer: public Ui_DlgRegisterPeer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGREGISTERPEER_H
