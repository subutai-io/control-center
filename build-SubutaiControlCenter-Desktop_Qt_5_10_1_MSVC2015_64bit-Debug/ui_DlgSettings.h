/********************************************************************************
** Form generated from reading UI file 'DlgSettings.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLGSETTINGS_H
#define UI_DLGSETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DlgSettings
{
public:
    QVBoxLayout *verticalLayout_4;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout_3;
    QPushButton *btn_cancel;
    QPushButton *btn_ok;
    QTabWidget *tabWidget;
    QWidget *tab_common;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout_6;
    QLabel *lbl_log_level;
    QGridLayout *gridLayout_2;
    QLabel *lbl_ssh_user;
    QLineEdit *le_ssh_user;
    QLabel *lbl_err_ssh_user;
    QLabel *lbl_notification_delay;
    QComboBox *cb_log_level;
    QGridLayout *gridLayout_4;
    QLabel *lbl_logs_storage;
    QPushButton *btn_logs_storage;
    QLabel *lbl_err_logs_storage;
    QPushButton *btn_ssh_keys_storage;
    QLabel *lbl_err_ssh_keys_storage;
    QLineEdit *le_logs_storage;
    QLineEdit *le_ssh_keys_storage;
    QLabel *lbl_ssh_keys_storage;
    QComboBox *cb_preferred_notifications_place;
    QComboBox *cb_notification_level;
    QLabel *lbl_notification_level;
    QCheckBox *chk_use_animations;
    QLabel *lbl_refresh_timer;
    QSpinBox *sb_notification_delay;
    QSpinBox *sb_refresh_timeout;
    QComboBox *cb_locale;
    QLabel *lbl_locale;
    QLabel *lbl_preferred_notifications_place;
    QCheckBox *chk_autostart;
    QLabel *lbl_tray_skin;
    QComboBox *cb_tray_skin;
    QWidget *tab_commands;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *lbl_err_p2p_command;
    QPushButton *btn_x2goclient_command;
    QPushButton *btn_vagrant_command;
    QLabel *label_3;
    QLabel *lbl_err_scp_command;
    QPushButton *btn_ssh_command;
    QGroupBox *gb_terminal_settings;
    QVBoxLayout *verticalLayout_6;
    QGridLayout *gridLayout_10;
    QLabel *lbl_terminal_cmd;
    QLineEdit *le_terminal_cmd;
    QLabel *lbl_ssh_command_2;
    QLineEdit *le_terminal_arg;
    QLabel *lbl_err_terminal_cmd;
    QLabel *lbl_err_terminal_arg;
    QLabel *lbl_err_vagrant_command;
    QLabel *lbl_err_ssh_command;
    QLineEdit *le_ssh_command;
    QLabel *label_31;
    QLabel *lbl_ssh_command;
    QLineEdit *le_x2goclient_command;
    QLineEdit *le_vagrant_command;
    QLabel *lbl_err_ssh_keygen_command;
    QLabel *lbl_p2p_path;
    QLineEdit *le_scp_command;
    QLineEdit *le_p2p_command;
    QLabel *label_2;
    QLabel *label;
    QPushButton *btn_p2p_file_dialog;
    QLineEdit *le_ssh_keygen_command;
    QLabel *lbl_err_x2goclient_command;
    QPushButton *btn_ssh_keygen_command;
    QPushButton *btn_scp_command;
    QLineEdit *le_virtualbox_command;
    QLabel *lbl_virtualbox_command;
    QPushButton *btn_virtualbox_command;
    QLabel *lbl_err_virtualbox_command;
    QWidget *tab_rh_ip;
    QVBoxLayout *verticalLayout_5;
    QGridLayout *gridLayout_11;
    QLabel *lbl_rhip_host;
    QLabel *lbl_rh_list;
    QListView *lstv_resource_hosts;
    QGridLayout *gridLayout_8;
    QPushButton *btn_refresh_rh_list;
    QProgressBar *pb_refresh_rh_list;
    QLabel *lbl_rhip_port;
    QLabel *lbl_rhip_user;
    QLabel *lbl_rhip_pass;
    QLineEdit *le_rhip_user;
    QLineEdit *le_rhip_password;
    QLabel *lbl_err_rhip_host;
    QLabel *lbl_err_rhip_user;
    QLineEdit *le_rhip_host;
    QLineEdit *le_rhip_port;
    QLabel *lbl_err_resource_hosts;
    QLabel *lbl_err_rhip_port;
    QLabel *lbl_err_rhip_password;
    QWidget *tab_updating;
    QVBoxLayout *verticalLayout_3;
    QGridLayout *gridLayout_9;
    QLabel *lbl_rh_update;
    QLabel *lbl_tray_update;
    QComboBox *cb_tray_frequency;
    QComboBox *cb_rh_frequency;
    QCheckBox *chk_tray_autoupdate;
    QCheckBox *chk_rh_autoupdate;
    QLabel *lbl_p2p_update;
    QComboBox *cb_p2p_frequency;
    QCheckBox *chk_p2p_autoupdate;
    QLabel *lbl_rhm_update;
    QComboBox *cb_rhm_frequency;
    QCheckBox *chk_rhm_autoupdate;

    void setupUi(QDialog *DlgSettings)
    {
        if (DlgSettings->objectName().isEmpty())
            DlgSettings->setObjectName(QStringLiteral("DlgSettings"));
        DlgSettings->resize(622, 632);
        QIcon icon;
        icon.addFile(QStringLiteral(":/hub/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
        DlgSettings->setWindowIcon(icon);
        DlgSettings->setModal(false);
        verticalLayout_4 = new QVBoxLayout(DlgSettings);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        btn_cancel = new QPushButton(DlgSettings);
        btn_cancel->setObjectName(QStringLiteral("btn_cancel"));

        gridLayout_3->addWidget(btn_cancel, 0, 1, 1, 1);

        btn_ok = new QPushButton(DlgSettings);
        btn_ok->setObjectName(QStringLiteral("btn_ok"));

        gridLayout_3->addWidget(btn_ok, 0, 0, 1, 1);


        gridLayout_5->addLayout(gridLayout_3, 1, 0, 1, 1);

        tabWidget = new QTabWidget(DlgSettings);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab_common = new QWidget();
        tab_common->setObjectName(QStringLiteral("tab_common"));
        verticalLayout_2 = new QVBoxLayout(tab_common);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        gridLayout_6 = new QGridLayout();
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        lbl_log_level = new QLabel(tab_common);
        lbl_log_level->setObjectName(QStringLiteral("lbl_log_level"));

        gridLayout_6->addWidget(lbl_log_level, 4, 0, 1, 1);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        lbl_ssh_user = new QLabel(tab_common);
        lbl_ssh_user->setObjectName(QStringLiteral("lbl_ssh_user"));

        gridLayout_2->addWidget(lbl_ssh_user, 0, 0, 1, 1);

        le_ssh_user = new QLineEdit(tab_common);
        le_ssh_user->setObjectName(QStringLiteral("le_ssh_user"));

        gridLayout_2->addWidget(le_ssh_user, 0, 1, 1, 1);

        lbl_err_ssh_user = new QLabel(tab_common);
        lbl_err_ssh_user->setObjectName(QStringLiteral("lbl_err_ssh_user"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lbl_err_ssh_user->sizePolicy().hasHeightForWidth());
        lbl_err_ssh_user->setSizePolicy(sizePolicy);
        lbl_err_ssh_user->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(lbl_err_ssh_user, 1, 1, 1, 1);


        gridLayout_6->addLayout(gridLayout_2, 9, 0, 1, 3);

        lbl_notification_delay = new QLabel(tab_common);
        lbl_notification_delay->setObjectName(QStringLiteral("lbl_notification_delay"));

        gridLayout_6->addWidget(lbl_notification_delay, 2, 0, 1, 1);

        cb_log_level = new QComboBox(tab_common);
        cb_log_level->setObjectName(QStringLiteral("cb_log_level"));

        gridLayout_6->addWidget(cb_log_level, 4, 2, 1, 1);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        lbl_logs_storage = new QLabel(tab_common);
        lbl_logs_storage->setObjectName(QStringLiteral("lbl_logs_storage"));

        gridLayout_4->addWidget(lbl_logs_storage, 0, 0, 1, 1);

        btn_logs_storage = new QPushButton(tab_common);
        btn_logs_storage->setObjectName(QStringLiteral("btn_logs_storage"));

        gridLayout_4->addWidget(btn_logs_storage, 0, 3, 1, 1);

        lbl_err_logs_storage = new QLabel(tab_common);
        lbl_err_logs_storage->setObjectName(QStringLiteral("lbl_err_logs_storage"));
        sizePolicy.setHeightForWidth(lbl_err_logs_storage->sizePolicy().hasHeightForWidth());
        lbl_err_logs_storage->setSizePolicy(sizePolicy);
        lbl_err_logs_storage->setMinimumSize(QSize(0, 0));
        lbl_err_logs_storage->setMaximumSize(QSize(16777215, 16777215));
        lbl_err_logs_storage->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(lbl_err_logs_storage, 1, 2, 1, 1);

        btn_ssh_keys_storage = new QPushButton(tab_common);
        btn_ssh_keys_storage->setObjectName(QStringLiteral("btn_ssh_keys_storage"));

        gridLayout_4->addWidget(btn_ssh_keys_storage, 3, 3, 1, 1);

        lbl_err_ssh_keys_storage = new QLabel(tab_common);
        lbl_err_ssh_keys_storage->setObjectName(QStringLiteral("lbl_err_ssh_keys_storage"));
        sizePolicy.setHeightForWidth(lbl_err_ssh_keys_storage->sizePolicy().hasHeightForWidth());
        lbl_err_ssh_keys_storage->setSizePolicy(sizePolicy);
        lbl_err_ssh_keys_storage->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(lbl_err_ssh_keys_storage, 4, 2, 1, 1);

        le_logs_storage = new QLineEdit(tab_common);
        le_logs_storage->setObjectName(QStringLiteral("le_logs_storage"));
        le_logs_storage->setToolTipDuration(4);
        le_logs_storage->setMaxLength(32764);

        gridLayout_4->addWidget(le_logs_storage, 0, 2, 1, 1);

        le_ssh_keys_storage = new QLineEdit(tab_common);
        le_ssh_keys_storage->setObjectName(QStringLiteral("le_ssh_keys_storage"));

        gridLayout_4->addWidget(le_ssh_keys_storage, 3, 2, 1, 1);

        lbl_ssh_keys_storage = new QLabel(tab_common);
        lbl_ssh_keys_storage->setObjectName(QStringLiteral("lbl_ssh_keys_storage"));

        gridLayout_4->addWidget(lbl_ssh_keys_storage, 3, 0, 1, 1);


        gridLayout_6->addLayout(gridLayout_4, 8, 0, 1, 3);

        cb_preferred_notifications_place = new QComboBox(tab_common);
        cb_preferred_notifications_place->setObjectName(QStringLiteral("cb_preferred_notifications_place"));

        gridLayout_6->addWidget(cb_preferred_notifications_place, 5, 2, 1, 1);

        cb_notification_level = new QComboBox(tab_common);
        cb_notification_level->setObjectName(QStringLiteral("cb_notification_level"));

        gridLayout_6->addWidget(cb_notification_level, 3, 2, 1, 1);

        lbl_notification_level = new QLabel(tab_common);
        lbl_notification_level->setObjectName(QStringLiteral("lbl_notification_level"));

        gridLayout_6->addWidget(lbl_notification_level, 3, 0, 1, 1);

        chk_use_animations = new QCheckBox(tab_common);
        chk_use_animations->setObjectName(QStringLiteral("chk_use_animations"));

        gridLayout_6->addWidget(chk_use_animations, 7, 0, 1, 1);

        lbl_refresh_timer = new QLabel(tab_common);
        lbl_refresh_timer->setObjectName(QStringLiteral("lbl_refresh_timer"));

        gridLayout_6->addWidget(lbl_refresh_timer, 1, 0, 1, 1);

        sb_notification_delay = new QSpinBox(tab_common);
        sb_notification_delay->setObjectName(QStringLiteral("sb_notification_delay"));
        sb_notification_delay->setAlignment(Qt::AlignCenter);
        sb_notification_delay->setMinimum(3);
        sb_notification_delay->setMaximum(300);
        sb_notification_delay->setValue(5);

        gridLayout_6->addWidget(sb_notification_delay, 2, 2, 1, 1);

        sb_refresh_timeout = new QSpinBox(tab_common);
        sb_refresh_timeout->setObjectName(QStringLiteral("sb_refresh_timeout"));
        sb_refresh_timeout->setAlignment(Qt::AlignCenter);
        sb_refresh_timeout->setMinimum(10);
        sb_refresh_timeout->setMaximum(600);
        sb_refresh_timeout->setValue(120);

        gridLayout_6->addWidget(sb_refresh_timeout, 1, 2, 1, 1);

        cb_locale = new QComboBox(tab_common);
        cb_locale->setObjectName(QStringLiteral("cb_locale"));

        gridLayout_6->addWidget(cb_locale, 0, 2, 1, 1);

        lbl_locale = new QLabel(tab_common);
        lbl_locale->setObjectName(QStringLiteral("lbl_locale"));

        gridLayout_6->addWidget(lbl_locale, 0, 0, 1, 1);

        lbl_preferred_notifications_place = new QLabel(tab_common);
        lbl_preferred_notifications_place->setObjectName(QStringLiteral("lbl_preferred_notifications_place"));
        lbl_preferred_notifications_place->setWordWrap(false);

        gridLayout_6->addWidget(lbl_preferred_notifications_place, 5, 0, 1, 1);

        chk_autostart = new QCheckBox(tab_common);
        chk_autostart->setObjectName(QStringLiteral("chk_autostart"));

        gridLayout_6->addWidget(chk_autostart, 7, 2, 1, 1);

        lbl_tray_skin = new QLabel(tab_common);
        lbl_tray_skin->setObjectName(QStringLiteral("lbl_tray_skin"));

        gridLayout_6->addWidget(lbl_tray_skin, 6, 0, 1, 1);

        cb_tray_skin = new QComboBox(tab_common);
        cb_tray_skin->setObjectName(QStringLiteral("cb_tray_skin"));

        gridLayout_6->addWidget(cb_tray_skin, 6, 2, 1, 1);


        verticalLayout_2->addLayout(gridLayout_6);

        tabWidget->addTab(tab_common, QString());
        tab_commands = new QWidget();
        tab_commands->setObjectName(QStringLiteral("tab_commands"));
        verticalLayout = new QVBoxLayout(tab_commands);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        lbl_err_p2p_command = new QLabel(tab_commands);
        lbl_err_p2p_command->setObjectName(QStringLiteral("lbl_err_p2p_command"));
        sizePolicy.setHeightForWidth(lbl_err_p2p_command->sizePolicy().hasHeightForWidth());
        lbl_err_p2p_command->setSizePolicy(sizePolicy);
        lbl_err_p2p_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_p2p_command, 1, 1, 1, 1);

        btn_x2goclient_command = new QPushButton(tab_commands);
        btn_x2goclient_command->setObjectName(QStringLiteral("btn_x2goclient_command"));

        gridLayout->addWidget(btn_x2goclient_command, 6, 2, 1, 1);

        btn_vagrant_command = new QPushButton(tab_commands);
        btn_vagrant_command->setObjectName(QStringLiteral("btn_vagrant_command"));

        gridLayout->addWidget(btn_vagrant_command, 8, 2, 1, 1);

        label_3 = new QLabel(tab_commands);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 8, 0, 1, 1);

        lbl_err_scp_command = new QLabel(tab_commands);
        lbl_err_scp_command->setObjectName(QStringLiteral("lbl_err_scp_command"));
        sizePolicy.setHeightForWidth(lbl_err_scp_command->sizePolicy().hasHeightForWidth());
        lbl_err_scp_command->setSizePolicy(sizePolicy);
        lbl_err_scp_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_scp_command, 13, 1, 1, 1);

        btn_ssh_command = new QPushButton(tab_commands);
        btn_ssh_command->setObjectName(QStringLiteral("btn_ssh_command"));

        gridLayout->addWidget(btn_ssh_command, 2, 2, 1, 1);

        gb_terminal_settings = new QGroupBox(tab_commands);
        gb_terminal_settings->setObjectName(QStringLiteral("gb_terminal_settings"));
        verticalLayout_6 = new QVBoxLayout(gb_terminal_settings);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        gridLayout_10 = new QGridLayout();
        gridLayout_10->setObjectName(QStringLiteral("gridLayout_10"));
        lbl_terminal_cmd = new QLabel(gb_terminal_settings);
        lbl_terminal_cmd->setObjectName(QStringLiteral("lbl_terminal_cmd"));

        gridLayout_10->addWidget(lbl_terminal_cmd, 0, 0, 1, 1);

        le_terminal_cmd = new QLineEdit(gb_terminal_settings);
        le_terminal_cmd->setObjectName(QStringLiteral("le_terminal_cmd"));

        gridLayout_10->addWidget(le_terminal_cmd, 0, 1, 1, 1);

        lbl_ssh_command_2 = new QLabel(gb_terminal_settings);
        lbl_ssh_command_2->setObjectName(QStringLiteral("lbl_ssh_command_2"));

        gridLayout_10->addWidget(lbl_ssh_command_2, 2, 0, 1, 1);

        le_terminal_arg = new QLineEdit(gb_terminal_settings);
        le_terminal_arg->setObjectName(QStringLiteral("le_terminal_arg"));

        gridLayout_10->addWidget(le_terminal_arg, 2, 1, 1, 1);

        lbl_err_terminal_cmd = new QLabel(gb_terminal_settings);
        lbl_err_terminal_cmd->setObjectName(QStringLiteral("lbl_err_terminal_cmd"));
        sizePolicy.setHeightForWidth(lbl_err_terminal_cmd->sizePolicy().hasHeightForWidth());
        lbl_err_terminal_cmd->setSizePolicy(sizePolicy);
        lbl_err_terminal_cmd->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_10->addWidget(lbl_err_terminal_cmd, 1, 1, 1, 1);

        lbl_err_terminal_arg = new QLabel(gb_terminal_settings);
        lbl_err_terminal_arg->setObjectName(QStringLiteral("lbl_err_terminal_arg"));
        sizePolicy.setHeightForWidth(lbl_err_terminal_arg->sizePolicy().hasHeightForWidth());
        lbl_err_terminal_arg->setSizePolicy(sizePolicy);
        lbl_err_terminal_arg->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_10->addWidget(lbl_err_terminal_arg, 3, 1, 1, 1);


        verticalLayout_6->addLayout(gridLayout_10);


        gridLayout->addWidget(gb_terminal_settings, 18, 0, 1, 3);

        lbl_err_vagrant_command = new QLabel(tab_commands);
        lbl_err_vagrant_command->setObjectName(QStringLiteral("lbl_err_vagrant_command"));
        sizePolicy.setHeightForWidth(lbl_err_vagrant_command->sizePolicy().hasHeightForWidth());
        lbl_err_vagrant_command->setSizePolicy(sizePolicy);
        lbl_err_vagrant_command->setLayoutDirection(Qt::LeftToRight);
        lbl_err_vagrant_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_vagrant_command, 9, 1, 1, 1);

        lbl_err_ssh_command = new QLabel(tab_commands);
        lbl_err_ssh_command->setObjectName(QStringLiteral("lbl_err_ssh_command"));
        sizePolicy.setHeightForWidth(lbl_err_ssh_command->sizePolicy().hasHeightForWidth());
        lbl_err_ssh_command->setSizePolicy(sizePolicy);
        lbl_err_ssh_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_ssh_command, 3, 1, 1, 1);

        le_ssh_command = new QLineEdit(tab_commands);
        le_ssh_command->setObjectName(QStringLiteral("le_ssh_command"));

        gridLayout->addWidget(le_ssh_command, 2, 1, 1, 1);

        label_31 = new QLabel(tab_commands);
        label_31->setObjectName(QStringLiteral("label_31"));

        gridLayout->addWidget(label_31, 11, 0, 1, 1);

        lbl_ssh_command = new QLabel(tab_commands);
        lbl_ssh_command->setObjectName(QStringLiteral("lbl_ssh_command"));

        gridLayout->addWidget(lbl_ssh_command, 2, 0, 1, 1);

        le_x2goclient_command = new QLineEdit(tab_commands);
        le_x2goclient_command->setObjectName(QStringLiteral("le_x2goclient_command"));

        gridLayout->addWidget(le_x2goclient_command, 6, 1, 1, 1);

        le_vagrant_command = new QLineEdit(tab_commands);
        le_vagrant_command->setObjectName(QStringLiteral("le_vagrant_command"));

        gridLayout->addWidget(le_vagrant_command, 8, 1, 1, 1);

        lbl_err_ssh_keygen_command = new QLabel(tab_commands);
        lbl_err_ssh_keygen_command->setObjectName(QStringLiteral("lbl_err_ssh_keygen_command"));
        sizePolicy.setHeightForWidth(lbl_err_ssh_keygen_command->sizePolicy().hasHeightForWidth());
        lbl_err_ssh_keygen_command->setSizePolicy(sizePolicy);
        lbl_err_ssh_keygen_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_ssh_keygen_command, 5, 1, 1, 1);

        lbl_p2p_path = new QLabel(tab_commands);
        lbl_p2p_path->setObjectName(QStringLiteral("lbl_p2p_path"));

        gridLayout->addWidget(lbl_p2p_path, 0, 0, 1, 1);

        le_scp_command = new QLineEdit(tab_commands);
        le_scp_command->setObjectName(QStringLiteral("le_scp_command"));

        gridLayout->addWidget(le_scp_command, 11, 1, 1, 1);

        le_p2p_command = new QLineEdit(tab_commands);
        le_p2p_command->setObjectName(QStringLiteral("le_p2p_command"));

        gridLayout->addWidget(le_p2p_command, 0, 1, 1, 1);

        label_2 = new QLabel(tab_commands);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 6, 0, 1, 1);

        label = new QLabel(tab_commands);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 4, 0, 1, 1);

        btn_p2p_file_dialog = new QPushButton(tab_commands);
        btn_p2p_file_dialog->setObjectName(QStringLiteral("btn_p2p_file_dialog"));

        gridLayout->addWidget(btn_p2p_file_dialog, 0, 2, 1, 1);

        le_ssh_keygen_command = new QLineEdit(tab_commands);
        le_ssh_keygen_command->setObjectName(QStringLiteral("le_ssh_keygen_command"));

        gridLayout->addWidget(le_ssh_keygen_command, 4, 1, 1, 1);

        lbl_err_x2goclient_command = new QLabel(tab_commands);
        lbl_err_x2goclient_command->setObjectName(QStringLiteral("lbl_err_x2goclient_command"));
        sizePolicy.setHeightForWidth(lbl_err_x2goclient_command->sizePolicy().hasHeightForWidth());
        lbl_err_x2goclient_command->setSizePolicy(sizePolicy);
        lbl_err_x2goclient_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_x2goclient_command, 7, 1, 1, 1);

        btn_ssh_keygen_command = new QPushButton(tab_commands);
        btn_ssh_keygen_command->setObjectName(QStringLiteral("btn_ssh_keygen_command"));

        gridLayout->addWidget(btn_ssh_keygen_command, 4, 2, 1, 1);

        btn_scp_command = new QPushButton(tab_commands);
        btn_scp_command->setObjectName(QStringLiteral("btn_scp_command"));

        gridLayout->addWidget(btn_scp_command, 11, 2, 1, 1);

        le_virtualbox_command = new QLineEdit(tab_commands);
        le_virtualbox_command->setObjectName(QStringLiteral("le_virtualbox_command"));

        gridLayout->addWidget(le_virtualbox_command, 14, 1, 1, 1);

        lbl_virtualbox_command = new QLabel(tab_commands);
        lbl_virtualbox_command->setObjectName(QStringLiteral("lbl_virtualbox_command"));

        gridLayout->addWidget(lbl_virtualbox_command, 14, 0, 1, 1);

        btn_virtualbox_command = new QPushButton(tab_commands);
        btn_virtualbox_command->setObjectName(QStringLiteral("btn_virtualbox_command"));

        gridLayout->addWidget(btn_virtualbox_command, 14, 2, 1, 1);

        lbl_err_virtualbox_command = new QLabel(tab_commands);
        lbl_err_virtualbox_command->setObjectName(QStringLiteral("lbl_err_virtualbox_command"));
        sizePolicy.setHeightForWidth(lbl_err_virtualbox_command->sizePolicy().hasHeightForWidth());
        lbl_err_virtualbox_command->setSizePolicy(sizePolicy);
        lbl_err_virtualbox_command->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lbl_err_virtualbox_command, 15, 1, 1, 1);


        verticalLayout->addLayout(gridLayout);

        tabWidget->addTab(tab_commands, QString());
        tab_rh_ip = new QWidget();
        tab_rh_ip->setObjectName(QStringLiteral("tab_rh_ip"));
        verticalLayout_5 = new QVBoxLayout(tab_rh_ip);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        gridLayout_11 = new QGridLayout();
        gridLayout_11->setObjectName(QStringLiteral("gridLayout_11"));
        lbl_rhip_host = new QLabel(tab_rh_ip);
        lbl_rhip_host->setObjectName(QStringLiteral("lbl_rhip_host"));

        gridLayout_11->addWidget(lbl_rhip_host, 0, 0, 1, 1);

        lbl_rh_list = new QLabel(tab_rh_ip);
        lbl_rh_list->setObjectName(QStringLiteral("lbl_rh_list"));

        gridLayout_11->addWidget(lbl_rh_list, 2, 0, 1, 1);

        lstv_resource_hosts = new QListView(tab_rh_ip);
        lstv_resource_hosts->setObjectName(QStringLiteral("lstv_resource_hosts"));

        gridLayout_11->addWidget(lstv_resource_hosts, 2, 1, 1, 1);

        gridLayout_8 = new QGridLayout();
        gridLayout_8->setObjectName(QStringLiteral("gridLayout_8"));
        btn_refresh_rh_list = new QPushButton(tab_rh_ip);
        btn_refresh_rh_list->setObjectName(QStringLiteral("btn_refresh_rh_list"));

        gridLayout_8->addWidget(btn_refresh_rh_list, 0, 0, 1, 1);

        pb_refresh_rh_list = new QProgressBar(tab_rh_ip);
        pb_refresh_rh_list->setObjectName(QStringLiteral("pb_refresh_rh_list"));
        pb_refresh_rh_list->setValue(0);

        gridLayout_8->addWidget(pb_refresh_rh_list, 1, 0, 1, 1);


        gridLayout_11->addLayout(gridLayout_8, 2, 2, 1, 1);

        lbl_rhip_port = new QLabel(tab_rh_ip);
        lbl_rhip_port->setObjectName(QStringLiteral("lbl_rhip_port"));

        gridLayout_11->addWidget(lbl_rhip_port, 4, 0, 1, 1);

        lbl_rhip_user = new QLabel(tab_rh_ip);
        lbl_rhip_user->setObjectName(QStringLiteral("lbl_rhip_user"));

        gridLayout_11->addWidget(lbl_rhip_user, 6, 0, 1, 1);

        lbl_rhip_pass = new QLabel(tab_rh_ip);
        lbl_rhip_pass->setObjectName(QStringLiteral("lbl_rhip_pass"));

        gridLayout_11->addWidget(lbl_rhip_pass, 8, 0, 1, 1);

        le_rhip_user = new QLineEdit(tab_rh_ip);
        le_rhip_user->setObjectName(QStringLiteral("le_rhip_user"));

        gridLayout_11->addWidget(le_rhip_user, 6, 1, 1, 2);

        le_rhip_password = new QLineEdit(tab_rh_ip);
        le_rhip_password->setObjectName(QStringLiteral("le_rhip_password"));
        le_rhip_password->setEchoMode(QLineEdit::Password);

        gridLayout_11->addWidget(le_rhip_password, 8, 1, 1, 2);

        lbl_err_rhip_host = new QLabel(tab_rh_ip);
        lbl_err_rhip_host->setObjectName(QStringLiteral("lbl_err_rhip_host"));
        sizePolicy.setHeightForWidth(lbl_err_rhip_host->sizePolicy().hasHeightForWidth());
        lbl_err_rhip_host->setSizePolicy(sizePolicy);
        lbl_err_rhip_host->setFocusPolicy(Qt::StrongFocus);
        lbl_err_rhip_host->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_11->addWidget(lbl_err_rhip_host, 1, 1, 1, 2);

        lbl_err_rhip_user = new QLabel(tab_rh_ip);
        lbl_err_rhip_user->setObjectName(QStringLiteral("lbl_err_rhip_user"));
        sizePolicy.setHeightForWidth(lbl_err_rhip_user->sizePolicy().hasHeightForWidth());
        lbl_err_rhip_user->setSizePolicy(sizePolicy);
        lbl_err_rhip_user->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_11->addWidget(lbl_err_rhip_user, 7, 1, 1, 2);

        le_rhip_host = new QLineEdit(tab_rh_ip);
        le_rhip_host->setObjectName(QStringLiteral("le_rhip_host"));

        gridLayout_11->addWidget(le_rhip_host, 0, 1, 1, 2);

        le_rhip_port = new QLineEdit(tab_rh_ip);
        le_rhip_port->setObjectName(QStringLiteral("le_rhip_port"));

        gridLayout_11->addWidget(le_rhip_port, 4, 1, 1, 2);

        lbl_err_resource_hosts = new QLabel(tab_rh_ip);
        lbl_err_resource_hosts->setObjectName(QStringLiteral("lbl_err_resource_hosts"));
        sizePolicy.setHeightForWidth(lbl_err_resource_hosts->sizePolicy().hasHeightForWidth());
        lbl_err_resource_hosts->setSizePolicy(sizePolicy);
        lbl_err_resource_hosts->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_11->addWidget(lbl_err_resource_hosts, 3, 1, 1, 1);

        lbl_err_rhip_port = new QLabel(tab_rh_ip);
        lbl_err_rhip_port->setObjectName(QStringLiteral("lbl_err_rhip_port"));
        sizePolicy.setHeightForWidth(lbl_err_rhip_port->sizePolicy().hasHeightForWidth());
        lbl_err_rhip_port->setSizePolicy(sizePolicy);
        lbl_err_rhip_port->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_11->addWidget(lbl_err_rhip_port, 5, 1, 1, 2);

        lbl_err_rhip_password = new QLabel(tab_rh_ip);
        lbl_err_rhip_password->setObjectName(QStringLiteral("lbl_err_rhip_password"));
        sizePolicy.setHeightForWidth(lbl_err_rhip_password->sizePolicy().hasHeightForWidth());
        lbl_err_rhip_password->setSizePolicy(sizePolicy);
        lbl_err_rhip_password->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_11->addWidget(lbl_err_rhip_password, 9, 1, 1, 2);


        verticalLayout_5->addLayout(gridLayout_11);

        tabWidget->addTab(tab_rh_ip, QString());
        tab_updating = new QWidget();
        tab_updating->setObjectName(QStringLiteral("tab_updating"));
        verticalLayout_3 = new QVBoxLayout(tab_updating);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        gridLayout_9 = new QGridLayout();
        gridLayout_9->setObjectName(QStringLiteral("gridLayout_9"));
        lbl_rh_update = new QLabel(tab_updating);
        lbl_rh_update->setObjectName(QStringLiteral("lbl_rh_update"));

        gridLayout_9->addWidget(lbl_rh_update, 2, 0, 1, 1);

        lbl_tray_update = new QLabel(tab_updating);
        lbl_tray_update->setObjectName(QStringLiteral("lbl_tray_update"));

        gridLayout_9->addWidget(lbl_tray_update, 1, 0, 1, 1);

        cb_tray_frequency = new QComboBox(tab_updating);
        cb_tray_frequency->setObjectName(QStringLiteral("cb_tray_frequency"));

        gridLayout_9->addWidget(cb_tray_frequency, 1, 1, 1, 1);

        cb_rh_frequency = new QComboBox(tab_updating);
        cb_rh_frequency->setObjectName(QStringLiteral("cb_rh_frequency"));

        gridLayout_9->addWidget(cb_rh_frequency, 2, 1, 1, 1);

        chk_tray_autoupdate = new QCheckBox(tab_updating);
        chk_tray_autoupdate->setObjectName(QStringLiteral("chk_tray_autoupdate"));

        gridLayout_9->addWidget(chk_tray_autoupdate, 1, 2, 1, 1);

        chk_rh_autoupdate = new QCheckBox(tab_updating);
        chk_rh_autoupdate->setObjectName(QStringLiteral("chk_rh_autoupdate"));

        gridLayout_9->addWidget(chk_rh_autoupdate, 2, 2, 1, 1);

        lbl_p2p_update = new QLabel(tab_updating);
        lbl_p2p_update->setObjectName(QStringLiteral("lbl_p2p_update"));

        gridLayout_9->addWidget(lbl_p2p_update, 0, 0, 1, 1);

        cb_p2p_frequency = new QComboBox(tab_updating);
        cb_p2p_frequency->setObjectName(QStringLiteral("cb_p2p_frequency"));

        gridLayout_9->addWidget(cb_p2p_frequency, 0, 1, 1, 1);

        chk_p2p_autoupdate = new QCheckBox(tab_updating);
        chk_p2p_autoupdate->setObjectName(QStringLiteral("chk_p2p_autoupdate"));

        gridLayout_9->addWidget(chk_p2p_autoupdate, 0, 2, 1, 1);

        lbl_rhm_update = new QLabel(tab_updating);
        lbl_rhm_update->setObjectName(QStringLiteral("lbl_rhm_update"));

        gridLayout_9->addWidget(lbl_rhm_update, 3, 0, 1, 1);

        cb_rhm_frequency = new QComboBox(tab_updating);
        cb_rhm_frequency->setObjectName(QStringLiteral("cb_rhm_frequency"));

        gridLayout_9->addWidget(cb_rhm_frequency, 3, 1, 1, 1);

        chk_rhm_autoupdate = new QCheckBox(tab_updating);
        chk_rhm_autoupdate->setObjectName(QStringLiteral("chk_rhm_autoupdate"));

        gridLayout_9->addWidget(chk_rhm_autoupdate, 3, 2, 1, 1);

        gridLayout_9->setColumnStretch(0, 2);
        gridLayout_9->setColumnStretch(1, 1);
        gridLayout_9->setColumnStretch(2, 1);

        verticalLayout_3->addLayout(gridLayout_9);

        tabWidget->addTab(tab_updating, QString());

        gridLayout_5->addWidget(tabWidget, 0, 0, 1, 1);


        verticalLayout_4->addLayout(gridLayout_5);


        retranslateUi(DlgSettings);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DlgSettings);
    } // setupUi

    void retranslateUi(QDialog *DlgSettings)
    {
        DlgSettings->setWindowTitle(QApplication::translate("DlgSettings", "Settings", nullptr));
        btn_cancel->setText(QApplication::translate("DlgSettings", "Cancel", nullptr));
        btn_ok->setText(QApplication::translate("DlgSettings", "OK", nullptr));
        lbl_log_level->setText(QApplication::translate("DlgSettings", "Log level :", nullptr));
        lbl_ssh_user->setText(QApplication::translate("DlgSettings", "SSH User:", nullptr));
        lbl_err_ssh_user->setText(QString());
        lbl_notification_delay->setText(QApplication::translate("DlgSettings", "Notification delay (sec.) :", nullptr));
        lbl_logs_storage->setText(QApplication::translate("DlgSettings", "Logs storage:", nullptr));
        btn_logs_storage->setText(QApplication::translate("DlgSettings", "...", nullptr));
#ifndef QT_NO_WHATSTHIS
        lbl_err_logs_storage->setWhatsThis(QApplication::translate("DlgSettings", "<html><head/><body><p align=\"right\"><br/></p></body></html>", nullptr));
#endif // QT_NO_WHATSTHIS
        lbl_err_logs_storage->setText(QString());
        btn_ssh_keys_storage->setText(QApplication::translate("DlgSettings", "...", nullptr));
#ifndef QT_NO_WHATSTHIS
        lbl_err_ssh_keys_storage->setWhatsThis(QApplication::translate("DlgSettings", "<html><head/><body><p align=\"right\">mm,m,df</p></body></html>", nullptr));
#endif // QT_NO_WHATSTHIS
        lbl_err_ssh_keys_storage->setText(QString());
        lbl_ssh_keys_storage->setText(QApplication::translate("DlgSettings", "SSH-keys storage:", nullptr));
#ifndef QT_NO_TOOLTIP
        cb_preferred_notifications_place->setToolTip(QApplication::translate("DlgSettings", "Works only if tray couldn't get sys. tray icon's coordinates", nullptr));
#endif // QT_NO_TOOLTIP
        lbl_notification_level->setText(QApplication::translate("DlgSettings", "Notification level :", nullptr));
        chk_use_animations->setText(QApplication::translate("DlgSettings", "Use animations", nullptr));
        lbl_refresh_timer->setText(QApplication::translate("DlgSettings", "Refresh time (sec.) :", nullptr));
        lbl_locale->setText(QApplication::translate("DlgSettings", "Language", nullptr));
#ifndef QT_NO_TOOLTIP
        lbl_preferred_notifications_place->setToolTip(QApplication::translate("DlgSettings", "Works only if tray couldn't get sys. tray icon's coordinates", nullptr));
#endif // QT_NO_TOOLTIP
        lbl_preferred_notifications_place->setText(QApplication::translate("DlgSettings", "Area of notifications (only linux) :", nullptr));
        chk_autostart->setText(QApplication::translate("DlgSettings", "Start on login", nullptr));
        lbl_tray_skin->setText(QApplication::translate("DlgSettings", "Skin Theme:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_common), QApplication::translate("DlgSettings", "Common", nullptr));
        lbl_err_p2p_command->setText(QString());
        btn_x2goclient_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        btn_vagrant_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        label_3->setText(QApplication::translate("DlgSettings", "Vagrant command", nullptr));
        lbl_err_scp_command->setText(QString());
        btn_ssh_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        gb_terminal_settings->setTitle(QApplication::translate("DlgSettings", "Terminal settings:", nullptr));
        lbl_terminal_cmd->setText(QApplication::translate("DlgSettings", "Terminal:", nullptr));
        lbl_ssh_command_2->setText(QApplication::translate("DlgSettings", "Argument:", nullptr));
        lbl_err_terminal_cmd->setText(QString());
        lbl_err_terminal_arg->setText(QString());
        lbl_err_vagrant_command->setText(QString());
        lbl_err_ssh_command->setText(QString());
        label_31->setText(QApplication::translate("DlgSettings", "SCP command", nullptr));
        lbl_ssh_command->setText(QApplication::translate("DlgSettings", "SSH command:", nullptr));
        lbl_err_ssh_keygen_command->setText(QString());
        lbl_p2p_path->setText(QApplication::translate("DlgSettings", "P2P command:", nullptr));
        label_2->setText(QApplication::translate("DlgSettings", "X2Go-Client", nullptr));
        label->setText(QApplication::translate("DlgSettings", "Ssh-keygen:", nullptr));
        btn_p2p_file_dialog->setText(QApplication::translate("DlgSettings", "...", nullptr));
        lbl_err_x2goclient_command->setText(QString());
        btn_ssh_keygen_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        btn_scp_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        lbl_virtualbox_command->setText(QApplication::translate("DlgSettings", "Virtualbox command", nullptr));
        btn_virtualbox_command->setText(QApplication::translate("DlgSettings", "...", nullptr));
        lbl_err_virtualbox_command->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_commands), QApplication::translate("DlgSettings", "Commands", nullptr));
        lbl_rhip_host->setText(QApplication::translate("DlgSettings", "Default RH", nullptr));
        lbl_rh_list->setText(QApplication::translate("DlgSettings", "Available RHs", nullptr));
        btn_refresh_rh_list->setText(QApplication::translate("DlgSettings", "Search RHs", nullptr));
        lbl_rhip_port->setText(QApplication::translate("DlgSettings", "RH port:", nullptr));
        lbl_rhip_user->setText(QApplication::translate("DlgSettings", "RH user:", nullptr));
        lbl_rhip_pass->setText(QApplication::translate("DlgSettings", "RH password:", nullptr));
        lbl_err_rhip_host->setText(QString());
        lbl_err_rhip_user->setText(QString());
        lbl_err_resource_hosts->setText(QString());
        lbl_err_rhip_port->setText(QString());
        lbl_err_rhip_password->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tab_rh_ip), QApplication::translate("DlgSettings", "RH", nullptr));
        lbl_rh_update->setText(QApplication::translate("DlgSettings", "Resource Host check for updates frequency:", nullptr));
        lbl_tray_update->setText(QApplication::translate("DlgSettings", "Control Center check for updates frequency:", nullptr));
        chk_tray_autoupdate->setText(QApplication::translate("DlgSettings", "Automatic tray update", nullptr));
        chk_rh_autoupdate->setText(QApplication::translate("DlgSettings", "Automatic rh update", nullptr));
        lbl_p2p_update->setText(QApplication::translate("DlgSettings", "P2P check for updates frequency:", nullptr));
        chk_p2p_autoupdate->setText(QApplication::translate("DlgSettings", "Automatic p2p update", nullptr));
        lbl_rhm_update->setText(QApplication::translate("DlgSettings", "Management check for updates frequency:", nullptr));
        chk_rhm_autoupdate->setText(QApplication::translate("DlgSettings", "Automatic mh update", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_updating), QApplication::translate("DlgSettings", "Updating", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DlgSettings: public Ui_DlgSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLGSETTINGS_H
