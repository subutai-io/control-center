/********************************************************************************
** Form generated from reading UI file 'DlgLogin.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGLOGIN_H
#define UI_DLGLOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgLogin
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout_2;
    QPushButton *btn_ok;
    QLabel *lbl_register_link;
    QPushButton *btn_cancel;
    QGridLayout *gridLayout;
    QLabel *lbl_login;
    QLineEdit *le_login;
    QLabel *lbl_password;
    QLineEdit *le_password;
    QLabel *lbl_status;
    QGridLayout *gridLayout_4;
    QSpacerItem *horizontalSpacer_3;
    QCheckBox *cb_show_pass;
    QSpacerItem *horizontalSpacer_4;
    QGridLayout *gridLayout_3;
    QSpacerItem *horizontalSpacer;
    QCheckBox *cb_save_credentials;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btn_resolve;

    void setupUi(QDialog *DlgLogin)
    {
        if (DlgLogin->objectName().isEmpty())
            DlgLogin->setObjectName(QStringLiteral("DlgLogin"));
        DlgLogin->resize(477, 196);
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/log_in.png"), QSize(), QIcon::Normal, QIcon::Off);
        DlgLogin->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(DlgLogin);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        btn_ok = new QPushButton(DlgLogin);
        btn_ok->setObjectName(QStringLiteral("btn_ok"));

        gridLayout_2->addWidget(btn_ok, 0, 0, 1, 1);

        lbl_register_link = new QLabel(DlgLogin);
        lbl_register_link->setObjectName(QStringLiteral("lbl_register_link"));

        gridLayout_2->addWidget(lbl_register_link, 0, 1, 1, 1);

        btn_cancel = new QPushButton(DlgLogin);
        btn_cancel->setObjectName(QStringLiteral("btn_cancel"));

        gridLayout_2->addWidget(btn_cancel, 0, 2, 1, 1);


        gridLayout_5->addLayout(gridLayout_2, 3, 0, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lbl_login = new QLabel(DlgLogin);
        lbl_login->setObjectName(QStringLiteral("lbl_login"));

        gridLayout->addWidget(lbl_login, 0, 0, 1, 1);

        le_login = new QLineEdit(DlgLogin);
        le_login->setObjectName(QStringLiteral("le_login"));

        gridLayout->addWidget(le_login, 0, 1, 1, 1);

        lbl_password = new QLabel(DlgLogin);
        lbl_password->setObjectName(QStringLiteral("lbl_password"));

        gridLayout->addWidget(lbl_password, 1, 0, 1, 1);

        le_password = new QLineEdit(DlgLogin);
        le_password->setObjectName(QStringLiteral("le_password"));
        le_password->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(le_password, 1, 1, 1, 1);

        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(1, 3);

        gridLayout_5->addLayout(gridLayout, 0, 0, 1, 1);

        lbl_status = new QLabel(DlgLogin);
        lbl_status->setObjectName(QStringLiteral("lbl_status"));
        lbl_status->setTextFormat(Qt::RichText);
        lbl_status->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(lbl_status, 4, 0, 1, 1);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        horizontalSpacer_3 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_3, 0, 0, 1, 1);

        cb_show_pass = new QCheckBox(DlgLogin);
        cb_show_pass->setObjectName(QStringLiteral("cb_show_pass"));

        gridLayout_4->addWidget(cb_show_pass, 0, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_4, 0, 2, 1, 1);

        gridLayout_4->setColumnStretch(0, 2);
        gridLayout_4->setColumnStretch(1, 1);
        gridLayout_4->setColumnStretch(2, 2);

        gridLayout_5->addLayout(gridLayout_4, 1, 0, 1, 1);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        horizontalSpacer = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer, 0, 0, 1, 1);

        cb_save_credentials = new QCheckBox(DlgLogin);
        cb_save_credentials->setObjectName(QStringLiteral("cb_save_credentials"));

        gridLayout_3->addWidget(cb_save_credentials, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 2, 1, 1);

        gridLayout_3->setColumnStretch(0, 2);
        gridLayout_3->setColumnStretch(1, 1);
        gridLayout_3->setColumnStretch(2, 2);

        gridLayout_5->addLayout(gridLayout_3, 2, 0, 1, 1);

        btn_resolve = new QPushButton(DlgLogin);
        btn_resolve->setObjectName(QStringLiteral("btn_resolve"));

        gridLayout_5->addWidget(btn_resolve, 5, 0, 1, 1);


        verticalLayout->addLayout(gridLayout_5);


        retranslateUi(DlgLogin);

        QMetaObject::connectSlotsByName(DlgLogin);
    } // setupUi

    void retranslateUi(QDialog *DlgLogin)
    {
        DlgLogin->setWindowTitle(QApplication::translate("DlgLogin", "Bazaar Account Login", nullptr));
        btn_ok->setText(QApplication::translate("DlgLogin", "Sign in", nullptr));
        lbl_register_link->setText(QApplication::translate("DlgLogin", "Register", nullptr));
        btn_cancel->setText(QApplication::translate("DlgLogin", "Cancel", nullptr));
        lbl_login->setText(QApplication::translate("DlgLogin", "Username (or email):", nullptr));
        lbl_password->setText(QApplication::translate("DlgLogin", "Password:", nullptr));
        lbl_status->setText(QApplication::translate("DlgLogin", "lbl_status", nullptr));
        cb_show_pass->setText(QApplication::translate("DlgLogin", "Show password when type", nullptr));
        cb_save_credentials->setText(QApplication::translate("DlgLogin", "Remember me", nullptr));
        btn_resolve->setText(QApplication::translate("DlgLogin", "Resolve", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgLogin: public Ui_DlgLogin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGLOGIN_H
