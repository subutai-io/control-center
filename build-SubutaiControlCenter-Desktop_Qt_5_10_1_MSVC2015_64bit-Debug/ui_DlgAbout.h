/********************************************************************************
** Form generated from reading UI file 'DlgAbout.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGABOUT_H
#define UI_DLGABOUT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgAbout
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_3;
    QGridLayout *gridLayout;
    QProgressBar *pb_initialization_progress;
    QPushButton *btn_recheck;
    QLabel *lbl_about_init;
    QGridLayout *gridLayout_2;
    QLabel *lbl_tray_version_val;
    QLabel *lbl_rhm_version_val;
    QLabel *lbl_tray_version;
    QProgressBar *pb_rhm;
    QProgressBar *pb_tray;
    QLabel *lbl_x2go_version;
    QPushButton *btn_rhm_update;
    QProgressBar *pb_p2p;
    QLabel *lbl_p2p_version_val;
    QPushButton *btn_p2p_update;
    QProgressBar *pb_x2go;
    QPushButton *btn_x2go_update;
    QLabel *lbl_chrome_version_val;
    QLabel *lbl_rhm_version;
    QLabel *lbl_chrome_version;
    QPushButton *btn_tray_update;
    QPushButton *btn_rh_update;
    QPushButton *btn_vagrant_update;
    QLabel *lbl_vagrant_version;
    QLabel *lbl_rh_version_val;
    QProgressBar *pb_rh;
    QProgressBar *pb_vagrant;
    QLabel *lbl_vagrant_version_val;
    QLabel *lbl_x2go_version_val;
    QLabel *lbl_p2p_version;
    QLabel *lbl_rh_version;
    QLabel *lbl_oracle_virtualbox_version;
    QLabel *lbl_oracle_virtualbox_version_val;
    QPushButton *btn_oracle_virtualbox_update;
    QProgressBar *pb_oracle_virtualbox;

    void setupUi(QDialog *DlgAbout)
    {
        if (DlgAbout->objectName().isEmpty())
            DlgAbout->setObjectName(QStringLiteral("DlgAbout"));
        DlgAbout->resize(541, 289);
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/about.png"), QSize(), QIcon::Normal, QIcon::Off);
        DlgAbout->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(DlgAbout);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        pb_initialization_progress = new QProgressBar(DlgAbout);
        pb_initialization_progress->setObjectName(QStringLiteral("pb_initialization_progress"));
        pb_initialization_progress->setMaximum(7);
        pb_initialization_progress->setValue(0);

        gridLayout->addWidget(pb_initialization_progress, 0, 1, 1, 1);

        btn_recheck = new QPushButton(DlgAbout);
        btn_recheck->setObjectName(QStringLiteral("btn_recheck"));

        gridLayout->addWidget(btn_recheck, 0, 2, 1, 1);

        lbl_about_init = new QLabel(DlgAbout);
        lbl_about_init->setObjectName(QStringLiteral("lbl_about_init"));

        gridLayout->addWidget(lbl_about_init, 0, 0, 1, 1);

        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(1, 3);

        gridLayout_3->addLayout(gridLayout, 1, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        lbl_tray_version_val = new QLabel(DlgAbout);
        lbl_tray_version_val->setObjectName(QStringLiteral("lbl_tray_version_val"));
        lbl_tray_version_val->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbl_tray_version_val, 0, 1, 1, 1);

        lbl_rhm_version_val = new QLabel(DlgAbout);
        lbl_rhm_version_val->setObjectName(QStringLiteral("lbl_rhm_version_val"));
        lbl_rhm_version_val->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbl_rhm_version_val, 6, 1, 1, 1);

        lbl_tray_version = new QLabel(DlgAbout);
        lbl_tray_version->setObjectName(QStringLiteral("lbl_tray_version"));

        gridLayout_2->addWidget(lbl_tray_version, 0, 0, 1, 1);

        pb_rhm = new QProgressBar(DlgAbout);
        pb_rhm->setObjectName(QStringLiteral("pb_rhm"));
        pb_rhm->setEnabled(false);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pb_rhm->sizePolicy().hasHeightForWidth());
        pb_rhm->setSizePolicy(sizePolicy);
        pb_rhm->setValue(0);

        gridLayout_2->addWidget(pb_rhm, 6, 2, 1, 1);

        pb_tray = new QProgressBar(DlgAbout);
        pb_tray->setObjectName(QStringLiteral("pb_tray"));
        pb_tray->setEnabled(false);
        sizePolicy.setHeightForWidth(pb_tray->sizePolicy().hasHeightForWidth());
        pb_tray->setSizePolicy(sizePolicy);
        pb_tray->setValue(0);

        gridLayout_2->addWidget(pb_tray, 0, 2, 1, 1);

        lbl_x2go_version = new QLabel(DlgAbout);
        lbl_x2go_version->setObjectName(QStringLiteral("lbl_x2go_version"));

        gridLayout_2->addWidget(lbl_x2go_version, 3, 0, 1, 1);

        btn_rhm_update = new QPushButton(DlgAbout);
        btn_rhm_update->setObjectName(QStringLiteral("btn_rhm_update"));
        btn_rhm_update->setEnabled(false);
        sizePolicy.setHeightForWidth(btn_rhm_update->sizePolicy().hasHeightForWidth());
        btn_rhm_update->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(btn_rhm_update, 6, 3, 1, 1);

        pb_p2p = new QProgressBar(DlgAbout);
        pb_p2p->setObjectName(QStringLiteral("pb_p2p"));
        pb_p2p->setEnabled(false);
        sizePolicy.setHeightForWidth(pb_p2p->sizePolicy().hasHeightForWidth());
        pb_p2p->setSizePolicy(sizePolicy);
        pb_p2p->setValue(0);

        gridLayout_2->addWidget(pb_p2p, 1, 2, 1, 1);

        lbl_p2p_version_val = new QLabel(DlgAbout);
        lbl_p2p_version_val->setObjectName(QStringLiteral("lbl_p2p_version_val"));
        lbl_p2p_version_val->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbl_p2p_version_val, 1, 1, 1, 1);

        btn_p2p_update = new QPushButton(DlgAbout);
        btn_p2p_update->setObjectName(QStringLiteral("btn_p2p_update"));
        btn_p2p_update->setEnabled(false);
        sizePolicy.setHeightForWidth(btn_p2p_update->sizePolicy().hasHeightForWidth());
        btn_p2p_update->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(btn_p2p_update, 1, 3, 1, 1);

        pb_x2go = new QProgressBar(DlgAbout);
        pb_x2go->setObjectName(QStringLiteral("pb_x2go"));
        pb_x2go->setEnabled(false);
        pb_x2go->setValue(0);

        gridLayout_2->addWidget(pb_x2go, 3, 2, 1, 1);

        btn_x2go_update = new QPushButton(DlgAbout);
        btn_x2go_update->setObjectName(QStringLiteral("btn_x2go_update"));
        btn_x2go_update->setEnabled(false);

        gridLayout_2->addWidget(btn_x2go_update, 3, 3, 1, 1);

        lbl_chrome_version_val = new QLabel(DlgAbout);
        lbl_chrome_version_val->setObjectName(QStringLiteral("lbl_chrome_version_val"));
        lbl_chrome_version_val->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbl_chrome_version_val, 7, 1, 1, 1);

        lbl_rhm_version = new QLabel(DlgAbout);
        lbl_rhm_version->setObjectName(QStringLiteral("lbl_rhm_version"));

        gridLayout_2->addWidget(lbl_rhm_version, 6, 0, 1, 1);

        lbl_chrome_version = new QLabel(DlgAbout);
        lbl_chrome_version->setObjectName(QStringLiteral("lbl_chrome_version"));
        sizePolicy.setHeightForWidth(lbl_chrome_version->sizePolicy().hasHeightForWidth());
        lbl_chrome_version->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(lbl_chrome_version, 7, 0, 1, 1);

        btn_tray_update = new QPushButton(DlgAbout);
        btn_tray_update->setObjectName(QStringLiteral("btn_tray_update"));
        btn_tray_update->setEnabled(false);
        sizePolicy.setHeightForWidth(btn_tray_update->sizePolicy().hasHeightForWidth());
        btn_tray_update->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(btn_tray_update, 0, 3, 1, 1);

        btn_rh_update = new QPushButton(DlgAbout);
        btn_rh_update->setObjectName(QStringLiteral("btn_rh_update"));
        btn_rh_update->setEnabled(false);
        sizePolicy.setHeightForWidth(btn_rh_update->sizePolicy().hasHeightForWidth());
        btn_rh_update->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(btn_rh_update, 5, 3, 1, 1);

        btn_vagrant_update = new QPushButton(DlgAbout);
        btn_vagrant_update->setObjectName(QStringLiteral("btn_vagrant_update"));
        btn_vagrant_update->setEnabled(false);

        gridLayout_2->addWidget(btn_vagrant_update, 2, 3, 1, 1);

        lbl_vagrant_version = new QLabel(DlgAbout);
        lbl_vagrant_version->setObjectName(QStringLiteral("lbl_vagrant_version"));

        gridLayout_2->addWidget(lbl_vagrant_version, 2, 0, 1, 1);

        lbl_rh_version_val = new QLabel(DlgAbout);
        lbl_rh_version_val->setObjectName(QStringLiteral("lbl_rh_version_val"));
        lbl_rh_version_val->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout_2->addWidget(lbl_rh_version_val, 5, 1, 1, 1);

        pb_rh = new QProgressBar(DlgAbout);
        pb_rh->setObjectName(QStringLiteral("pb_rh"));
        pb_rh->setEnabled(false);
        sizePolicy.setHeightForWidth(pb_rh->sizePolicy().hasHeightForWidth());
        pb_rh->setSizePolicy(sizePolicy);
        pb_rh->setValue(0);

        gridLayout_2->addWidget(pb_rh, 5, 2, 1, 1);

        pb_vagrant = new QProgressBar(DlgAbout);
        pb_vagrant->setObjectName(QStringLiteral("pb_vagrant"));
        pb_vagrant->setEnabled(false);
        pb_vagrant->setValue(0);

        gridLayout_2->addWidget(pb_vagrant, 2, 2, 1, 1);

        lbl_vagrant_version_val = new QLabel(DlgAbout);
        lbl_vagrant_version_val->setObjectName(QStringLiteral("lbl_vagrant_version_val"));

        gridLayout_2->addWidget(lbl_vagrant_version_val, 2, 1, 1, 1);

        lbl_x2go_version_val = new QLabel(DlgAbout);
        lbl_x2go_version_val->setObjectName(QStringLiteral("lbl_x2go_version_val"));

        gridLayout_2->addWidget(lbl_x2go_version_val, 3, 1, 1, 1);

        lbl_p2p_version = new QLabel(DlgAbout);
        lbl_p2p_version->setObjectName(QStringLiteral("lbl_p2p_version"));

        gridLayout_2->addWidget(lbl_p2p_version, 1, 0, 1, 1);

        lbl_rh_version = new QLabel(DlgAbout);
        lbl_rh_version->setObjectName(QStringLiteral("lbl_rh_version"));

        gridLayout_2->addWidget(lbl_rh_version, 5, 0, 1, 1);

        lbl_oracle_virtualbox_version = new QLabel(DlgAbout);
        lbl_oracle_virtualbox_version->setObjectName(QStringLiteral("lbl_oracle_virtualbox_version"));

        gridLayout_2->addWidget(lbl_oracle_virtualbox_version, 4, 0, 1, 1);

        lbl_oracle_virtualbox_version_val = new QLabel(DlgAbout);
        lbl_oracle_virtualbox_version_val->setObjectName(QStringLiteral("lbl_oracle_virtualbox_version_val"));

        gridLayout_2->addWidget(lbl_oracle_virtualbox_version_val, 4, 1, 1, 1);

        btn_oracle_virtualbox_update = new QPushButton(DlgAbout);
        btn_oracle_virtualbox_update->setObjectName(QStringLiteral("btn_oracle_virtualbox_update"));
        btn_oracle_virtualbox_update->setEnabled(false);

        gridLayout_2->addWidget(btn_oracle_virtualbox_update, 4, 3, 1, 1);

        pb_oracle_virtualbox = new QProgressBar(DlgAbout);
        pb_oracle_virtualbox->setObjectName(QStringLiteral("pb_oracle_virtualbox"));
        pb_oracle_virtualbox->setEnabled(false);
        pb_oracle_virtualbox->setValue(0);

        gridLayout_2->addWidget(pb_oracle_virtualbox, 4, 2, 1, 1);

        gridLayout_2->setRowStretch(0, 2);
        gridLayout_2->setColumnStretch(0, 1);

        gridLayout_3->addLayout(gridLayout_2, 0, 0, 1, 1);

        gridLayout_3->setRowStretch(0, 10);

        verticalLayout->addLayout(gridLayout_3);


        retranslateUi(DlgAbout);

        QMetaObject::connectSlotsByName(DlgAbout);
    } // setupUi

    void retranslateUi(QDialog *DlgAbout)
    {
        DlgAbout->setWindowTitle(QApplication::translate("DlgAbout", "About subutai tray", nullptr));
        btn_recheck->setText(QApplication::translate("DlgAbout", "Recheck", nullptr));
        lbl_about_init->setText(QApplication::translate("DlgAbout", "Collecting data:", nullptr));
        lbl_tray_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_rhm_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_tray_version->setText(QApplication::translate("DlgAbout", "Control Center version:", nullptr));
        lbl_x2go_version->setText(QApplication::translate("DlgAbout", "X2Go-Client", nullptr));
        btn_rhm_update->setText(QApplication::translate("DlgAbout", "Update Management", nullptr));
        lbl_p2p_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        btn_p2p_update->setText(QApplication::translate("DlgAbout", "Update P2P", nullptr));
        btn_x2go_update->setText(QApplication::translate("DlgAbout", "Update X2Go-Client", nullptr));
        lbl_chrome_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_rhm_version->setText(QApplication::translate("DlgAbout", "Management version:", nullptr));
        lbl_chrome_version->setText(QApplication::translate("DlgAbout", "Chrome version:", nullptr));
        btn_tray_update->setText(QApplication::translate("DlgAbout", "Update Control Center", nullptr));
        btn_rh_update->setText(QApplication::translate("DlgAbout", "Update RH", nullptr));
        btn_vagrant_update->setText(QApplication::translate("DlgAbout", "Update Vagrant", nullptr));
        lbl_vagrant_version->setText(QApplication::translate("DlgAbout", "Vagrant", nullptr));
        lbl_rh_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_vagrant_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_x2go_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        lbl_p2p_version->setText(QApplication::translate("DlgAbout", "P2P version:", nullptr));
        lbl_rh_version->setText(QApplication::translate("DlgAbout", "RH version:", nullptr));
        lbl_oracle_virtualbox_version->setText(QApplication::translate("DlgAbout", "Oracle VirtualBox", nullptr));
        lbl_oracle_virtualbox_version_val->setText(QApplication::translate("DlgAbout", "undefined", nullptr));
        btn_oracle_virtualbox_update->setText(QApplication::translate("DlgAbout", "Update VirtualBox", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgAbout: public Ui_DlgAbout {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGABOUT_H
