/********************************************************************************
** Form generated from reading UI file 'DlgNotification.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGNOTIFICATION_H
#define UI_DLGNOTIFICATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_DlgNotification
{
public:
    QLabel *lbl_icon;
    QCheckBox *chk_autohide;
    QCheckBox *chk_ignore;
    QPushButton *btn_close;
    QPushButton *btn_action;
    QTextEdit *lbl_message;

    void setupUi(QDialog *DlgNotification)
    {
        if (DlgNotification->objectName().isEmpty())
            DlgNotification->setObjectName(QStringLiteral("DlgNotification"));
        DlgNotification->resize(600, 137);
        DlgNotification->setMinimumSize(QSize(600, 132));
        DlgNotification->setMaximumSize(QSize(600, 160));
        lbl_icon = new QLabel(DlgNotification);
        lbl_icon->setObjectName(QStringLiteral("lbl_icon"));
        lbl_icon->setGeometry(QRect(10, 10, 64, 64));
        lbl_icon->setAlignment(Qt::AlignCenter);
        chk_autohide = new QCheckBox(DlgNotification);
        chk_autohide->setObjectName(QStringLiteral("chk_autohide"));
        chk_autohide->setGeometry(QRect(170, 90, 161, 23));
        chk_ignore = new QCheckBox(DlgNotification);
        chk_ignore->setObjectName(QStringLiteral("chk_ignore"));
        chk_ignore->setGeometry(QRect(8, 90, 151, 23));
        btn_close = new QPushButton(DlgNotification);
        btn_close->setObjectName(QStringLiteral("btn_close"));
        btn_close->setGeometry(QRect(459, 90, 121, 25));
        btn_action = new QPushButton(DlgNotification);
        btn_action->setObjectName(QStringLiteral("btn_action"));
        btn_action->setGeometry(QRect(310, 90, 121, 25));
        lbl_message = new QTextEdit(DlgNotification);
        lbl_message->setObjectName(QStringLiteral("lbl_message"));
        lbl_message->setGeometry(QRect(90, 10, 491, 70));
        lbl_message->setReadOnly(true);
        lbl_message->setTextInteractionFlags(Qt::TextSelectableByMouse);

        retranslateUi(DlgNotification);

        QMetaObject::connectSlotsByName(DlgNotification);
    } // setupUi

    void retranslateUi(QDialog *DlgNotification)
    {
        DlgNotification->setWindowTitle(QApplication::translate("DlgNotification", "Dialog", nullptr));
        lbl_icon->setText(QApplication::translate("DlgNotification", "Icon", nullptr));
        chk_autohide->setText(QApplication::translate("DlgNotification", "Autohide", nullptr));
        chk_ignore->setText(QApplication::translate("DlgNotification", "Don't show again", nullptr));
        btn_close->setText(QApplication::translate("DlgNotification", "Close", nullptr));
        btn_action->setText(QApplication::translate("DlgNotification", "Update Resource Host", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgNotification: public Ui_DlgNotification {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGNOTIFICATION_H
