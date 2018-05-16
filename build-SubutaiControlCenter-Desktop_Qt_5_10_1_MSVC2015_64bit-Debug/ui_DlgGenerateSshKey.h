/********************************************************************************
** Form generated from reading UI file 'DlgGenerateSshKey.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGGENERATESSHKEY_H
#define UI_DLGGENERATESSHKEY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DlgGenerateSshKey
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_2;
    QLabel *lbl_enabled;
    QLabel *lbl_environments;
    QListView *lstv_sshkeys;
    QSpacerItem *horizontalSpacer;
    QGridLayout *gridLayout;
    QCheckBox *chk_select_all;
    QListView *lstv_environments;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btn_generate_new_key;
    QPushButton *btn_send_to_hub;
    QProgressBar *pb_send_to_hub;

    void setupUi(QDialog *DlgGenerateSshKey)
    {
        if (DlgGenerateSshKey->objectName().isEmpty())
            DlgGenerateSshKey->setObjectName(QStringLiteral("DlgGenerateSshKey"));
        DlgGenerateSshKey->resize(668, 328);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DlgGenerateSshKey->sizePolicy().hasHeightForWidth());
        DlgGenerateSshKey->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/ssh-keys.png"), QSize(), QIcon::Normal, QIcon::Off);
        DlgGenerateSshKey->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(DlgGenerateSshKey);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        lbl_enabled = new QLabel(DlgGenerateSshKey);
        lbl_enabled->setObjectName(QStringLiteral("lbl_enabled"));
        sizePolicy.setHeightForWidth(lbl_enabled->sizePolicy().hasHeightForWidth());
        lbl_enabled->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(lbl_enabled, 0, 0, 1, 1);

        lbl_environments = new QLabel(DlgGenerateSshKey);
        lbl_environments->setObjectName(QStringLiteral("lbl_environments"));
        sizePolicy.setHeightForWidth(lbl_environments->sizePolicy().hasHeightForWidth());
        lbl_environments->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(lbl_environments, 0, 2, 1, 1);

        lstv_sshkeys = new QListView(DlgGenerateSshKey);
        lstv_sshkeys->setObjectName(QStringLiteral("lstv_sshkeys"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lstv_sshkeys->sizePolicy().hasHeightForWidth());
        lstv_sshkeys->setSizePolicy(sizePolicy1);
        lstv_sshkeys->setAlternatingRowColors(true);
        lstv_sshkeys->setSelectionMode(QAbstractItemView::SingleSelection);

        gridLayout_2->addWidget(lstv_sshkeys, 1, 0, 2, 1);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 1, 1, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        chk_select_all = new QCheckBox(DlgGenerateSshKey);
        chk_select_all->setObjectName(QStringLiteral("chk_select_all"));

        gridLayout->addWidget(chk_select_all, 0, 0, 1, 1);

        lstv_environments = new QListView(DlgGenerateSshKey);
        lstv_environments->setObjectName(QStringLiteral("lstv_environments"));
        sizePolicy1.setHeightForWidth(lstv_environments->sizePolicy().hasHeightForWidth());
        lstv_environments->setSizePolicy(sizePolicy1);
        lstv_environments->setAlternatingRowColors(true);
        lstv_environments->setSelectionMode(QAbstractItemView::NoSelection);

        gridLayout->addWidget(lstv_environments, 1, 0, 1, 1);


        gridLayout_2->addLayout(gridLayout, 1, 2, 2, 1);

        horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 2, 1, 1, 1);

        btn_generate_new_key = new QPushButton(DlgGenerateSshKey);
        btn_generate_new_key->setObjectName(QStringLiteral("btn_generate_new_key"));

        gridLayout_2->addWidget(btn_generate_new_key, 4, 0, 1, 1);

        btn_send_to_hub = new QPushButton(DlgGenerateSshKey);
        btn_send_to_hub->setObjectName(QStringLiteral("btn_send_to_hub"));

        gridLayout_2->addWidget(btn_send_to_hub, 4, 2, 1, 1);

        pb_send_to_hub = new QProgressBar(DlgGenerateSshKey);
        pb_send_to_hub->setObjectName(QStringLiteral("pb_send_to_hub"));
        pb_send_to_hub->setValue(24);

        gridLayout_2->addWidget(pb_send_to_hub, 3, 0, 1, 3);


        verticalLayout->addLayout(gridLayout_2);


        retranslateUi(DlgGenerateSshKey);

        QMetaObject::connectSlotsByName(DlgGenerateSshKey);
    } // setupUi

    void retranslateUi(QDialog *DlgGenerateSshKey)
    {
        DlgGenerateSshKey->setWindowTitle(QApplication::translate("DlgGenerateSshKey", "SSH key generation", nullptr));
        lbl_enabled->setText(QApplication::translate("DlgGenerateSshKey", "Existing Keys:", nullptr));
        lbl_environments->setText(QApplication::translate("DlgGenerateSshKey", "Healthy Environments:", nullptr));
        chk_select_all->setText(QApplication::translate("DlgGenerateSshKey", "Select All To Deploy", nullptr));
        btn_generate_new_key->setText(QApplication::translate("DlgGenerateSshKey", "Generate New Key Pair", nullptr));
        btn_send_to_hub->setText(QApplication::translate("DlgGenerateSshKey", "Deploy Key[s]", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgGenerateSshKey: public Ui_DlgGenerateSshKey {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGGENERATESSHKEY_H
