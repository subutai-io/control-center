#include "DlgCreatePeer.h"
#include "DlgNotification.h"
#include "NotificationObserver.h"
#include "QDir"
#include "QStandardPaths"
#include "SettingsManager.h"
#include "SystemCallWrapper.h"
#include "TrayControlWindow.h"
#include "ui_DlgCreatePeer.h"
#include "Environment.h"

// Validator template for QlineEdit
template <class TC>
struct field_validator_t {
  TC* fc;  // field control
  QLabel* lbl_err;
  bool (*f_validator)(const TC*);
  QString validator_msg;
};

bool is_le_empty(const QLineEdit* le) {
  return le->text().trimmed().isEmpty();
}

// checks password size
bool is_le_valid_size(const QLineEdit* le) {
  if (le->text().size() < 7)
    return false;

  return true;
}

// checks password invalid characters
bool is_le_chars_and_digits(const QLineEdit* le) {
  QRegExp invalid_chars;
  invalid_chars.setPattern("\\W");

  if (le->text().contains(invalid_chars))
    return false;

  return true;
}

// checks only letters and numbers.
bool is_le_letter_and_number(const QLineEdit* le) {
  QRegExp invalid_chars;
  invalid_chars.setPattern("\\W");

  if (le->text().contains(invalid_chars) ||
      le->text().contains("_")) {
    return false;
  }

  return true;
}

// check min ram
bool is_le_has_min_ram(const QLineEdit* le) {
  if (le->text().toInt() < 2048)
    return false;

  return true;
}

// check max ram
bool is_le_has_max_ram(const QLineEdit* le) {
  if ((unsigned int)le->text().toInt() < Environment::Instance()->ramSize())
    return false;

  return true;
}

// check min disk
bool is_le_has_min_disk(const QLineEdit* le) {
  // min 100 gb disk size
  if (le->text().toInt() < 100)
    return false;

  return true;
}

// check max disk
bool is_le_has_max_disk(const QLineEdit* le) {
  // max 2 terebytes disk size
  if (le->text().toInt() <= 2048)
    return false;

  return true;
}

DlgCreatePeer::DlgCreatePeer(QWidget *parent)
    : QDialog(parent),
      m_password_state(0),
      m_password_confirm_state(0),
      ui(new Ui::DlgCreatePeer) {
  // ui
  ui->setupUi(this);
  ui->le_disk->setText("100");
  ui->cmb_bridge->addItems(CPeerController::Instance()->get_bridgedifs());
  hide_err_labels();
  this->adjustSize();
  // slots
  connect(ui->btn_cancel, &QPushButton::clicked, [this]() { this->close(); });
  connect(ui->btn_create, &QPushButton::clicked, this,
          &DlgCreatePeer::create_button_pressed);
  // requirements
  requirement virtualbox(
      tr("VirtualBox is not ready"), tr("Checking VirtualBox..."),
      tr("VirtualBox is not ready. You should install or update it from "
         "Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::ORACLE_VIRTUALBOX);
      });
  requirement vagrant(
      tr("Vagrant is not ready"), tr("Checking Vagrant..."),
      tr("Vagrant is not ready. You should install or update it from "
         "Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT);
      });
  requirement subutai_plugin(
      tr("Subutai plugin is not ready"), tr("Checking Subutai plugin..."),
      tr("Unable to run the Vagrant Subutai plugin. Make sure that you have it "
         "installed or updated successfully by going to the menu > "
         "Components."),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT_SUBUTAI);
      });
  requirement vbguest_plugin(
      tr("VirtualBox plugin is not ready"), tr("Checking VirtualBox plugin..."),
      tr("Vagrant VirtualBox plugin is not ready. You should install or update "
         "it from Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::VAGRANT_VBGUEST);
      });
  requirement subutai_box(
      tr("Subutai box is not ready"), tr("Checking Subutai box..."),
      tr("Vagrant Subutai box is not ready. You should install or update it "
         "from Components"),
      DlgNotification::N_ABOUT, []() {
        return !CHubComponentsUpdater::Instance()->is_update_available(
            IUpdaterComponent::SUBUTAI_BOX);
      });
  m_requirements_ls = std::vector<requirement>{
      vagrant, virtualbox, subutai_plugin, vbguest_plugin, subutai_box};
  // format
  ui->le_ram->setValidator(new QIntValidator(1, 100000, this));
  ui->le_disk->setValidator(new QIntValidator(1, 100000, this));
  ui->le_pass->setEchoMode(QLineEdit::Password);
  ui->le_pass_confirm->setEchoMode(QLineEdit::Password);
  ui->le_name->setMaxLength(20);
  ui->pb_peer->setMaximum(m_requirements_ls.size() + 1);
  m_invalid_chars.setPattern("\\W");
  static QIcon show_password_icon(":/hub/show-password.png");
  static QIcon hide_password_icon(":/hub/hide-password.png");

  // Password
  this->m_show_password_action =
      ui->le_pass->addAction(show_password_icon, QLineEdit::TrailingPosition);

  // Password Confirm
  this->m_show_confirm_password_action = ui->le_pass_confirm->addAction(
      show_password_icon, QLineEdit::TrailingPosition);

  // QLineEdit password show
  connect(this->m_show_password_action, &QAction::triggered, [this]() {
    this->m_password_state ^= 1;
    this->m_show_password_action->setIcon(
          this->m_password_state ? hide_password_icon : show_password_icon);
    ui->le_pass->setEchoMode(this->m_password_state ? QLineEdit::Normal
                                                    : QLineEdit::Password);
  });

  // QLineEdit confirm password
  connect(this->m_show_confirm_password_action, &QAction::triggered, [this]() {
    this->m_password_confirm_state ^= 1;
    this->m_show_confirm_password_action->setIcon(
          this->m_password_confirm_state ? show_password_icon : hide_password_icon);
    ui->le_pass_confirm->setEchoMode(this->m_password_confirm_state
                                         ? QLineEdit::Normal
                                         : QLineEdit::Password);
  });

  // add num cpu to combobox item
  for(unsigned int i = 0; i < Environment::Instance()->numCpu(); i++) {
    ui->cmb_cpu->addItem(QString::number(i+1));
  }
}

DlgCreatePeer::~DlgCreatePeer() { delete ui; }

DlgCreatePeer::pass_err DlgCreatePeer::check_pass(QString pass) {
  if (pass.isEmpty()) return PASS_EMPTY;  // empty error;
  if (pass.size() < 7)                    // too short
    return PASS_SMALL;
  if (pass.contains(m_invalid_chars))  // should contain chars and digits
    return PASS_INVALID;
  return PASS_FINE;
}

bool DlgCreatePeer::check_configurations() {
  QString ram = ui->le_ram->text();
  QString disk = ui->le_disk->text();
  QString password1 = ui->le_pass->text();
  QString password2 = ui->le_pass_confirm->text();
  bool errors_exist = false;
  // password
  pass_err pass_error = check_pass(password1);
  if (pass_error != 3) {
    QString error_message = "";
    switch (pass_error) {
      case PASS_EMPTY:
        error_message = tr("Password cannot be empty.");
        break;
      case PASS_SMALL:
        error_message = tr("Password size should be more than 7.");
        break;
      case PASS_INVALID:
        error_message = tr("Password has invalid symbols.");
        break;
      default:
        error_message =
            tr("An unexpected error has occurred. Please try again later.");
        break;
    }
    ui->lbl_err_pass->setText(error_message);
    ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_pass->show();
    errors_exist = true;
  } else if (password1 != password2) {
    ui->lbl_err_pass->setText(
        tr("Passwords do not match. Enter both passwords again."));
    ui->lbl_err_pass->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_pass->show();
    errors_exist = true;
  } else
    ui->lbl_err_pass->hide();
  // name
  if (ui->le_name->text().isEmpty()) {
    ui->lbl_err_name->setText(tr("Name of the Peer cannot be empty"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    errors_exist = true;
  } else if (ui->le_name->text().contains(m_invalid_chars) ||
             ui->le_name->text().contains("_")) {
    ui->lbl_err_name->setText(tr("Use only letters and numbers."));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    errors_exist = true;
  } else
    ui->lbl_err_name->hide();
  // ram
  if (ram.toInt() < 2048) {
    ui->lbl_err_ram->setText(tr("Ram cannot be less than 2048 MB."));
    ui->lbl_err_ram->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_ram->show();
    errors_exist = true;
  } else
    ui->lbl_err_ram->hide();
  // disk
  if (disk.toInt() < 40) {
    ui->lbl_err_disk->setText(tr("Disk cannot be less than 40 GB."));
    ui->lbl_err_disk->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_disk->show();
    errors_exist = true;
  } else
    ui->lbl_err_disk->hide();
  return errors_exist;
}

void DlgCreatePeer::create_button_pressed() {
  static const QString empty_validator_msg = tr("Field cannot be empty");
  static const  QString empty_password_msg = tr("Password cannot be empty.");
  static const QString password_size_msg = tr("Password size should be more than 7.");
  static const QString letters_or_digits_msg = tr("Use only letters and numbers.");
  static const QString min_ram_msg = tr("Ram cannot be less than 2048 MB.");
  static const QString max_ram_msg = tr("Ram cannot be more than %1 MB").arg(Environment::Instance()->ramSize());
  static const QString max_disk_msg = tr("Disk cannot be more than 2 TB.");
  static const QString min_disk_msg = tr("Disk cannot be less than 100 GB.");

  field_validator_t<QLineEdit> le_validators[] = {
    {ui->le_name, ui->lbl_err_name, is_le_empty, empty_validator_msg},
    {ui->le_pass, ui->lbl_err_pass, is_le_empty, empty_password_msg},
    {ui->le_pass, ui->lbl_err_pass, is_le_valid_size, password_size_msg},
    {ui->le_name, ui->lbl_err_name, is_le_letter_and_number, letters_or_digits_msg},
    {ui->le_ram, ui->lbl_err_ram, is_le_has_min_ram, min_ram_msg},
    {ui->le_ram, ui->lbl_err_ram, is_le_has_max_ram, max_ram_msg},
    {ui->le_disk, ui->lbl_err_disk, is_le_has_min_disk, min_disk_msg},
    {ui->le_disk, ui->lbl_err_disk, is_le_has_max_disk, max_disk_msg},
    {NULL, NULL, NULL, ""}
  };

  std::vector<field_validator_t<QLineEdit> > lst_failed_validators;
 // field_validator_t<QLineEdit>* tmp = le_validators;

  for(int8_t i = 0; i < (int8_t)5; i++) {
    qDebug() << "msg: " << le_validators[i].validator_msg;
    //if (!le_validators[i].fc->isVisible()) continue;
    //if (le_validators[i].f_validator(le_validators[i].fc)) continue;
    //lst_failed_validators.push_back(le_validators[i]);
  }

  qDebug() << "before crash " << lst_failed_validators.empty();


  if (check_configurations())
    return;

  qDebug() << "after return";
  QString dir = create_dir("subutai-peer_" + ui->le_name->text());
  if (dir.isEmpty()) {
    ui->lbl_err_name->setText(tr("Name already exists"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    ui->lbl_err_name->show();
    return;
  }
  hide_err_labels();
  set_enabled_buttons(false);
  if (!check_machine()) {
    ui->btn_create->setEnabled(true);
    ui->lbl_err_os->setStyleSheet("QLabel {color : red}");
    ui->pb_peer->setValue(0);
    ui->pb_peer->setEnabled(false);
    set_enabled_buttons(true);
    QDir directory_delete(dir);
    directory_delete.removeRecursively();
    return;
  }
  ui->lbl_err_os->setStyleSheet("QLabel {color : green}");
  ui->lbl_err_os->setText(tr("Initalializing environment..."));
  InitPeer *thread_init = new InitPeer(this);
  thread_init->init(dir, ui->cmb_os->currentText());
  thread_init->startWork();
  connect(thread_init, &InitPeer::outputReceived,
          [dir, this](system_call_wrapper_error_t res) {
            this->init_completed(res, dir, this->ui->le_ram->text(),
                                 this->ui->cmb_cpu->currentText(),
                                 this->ui->le_disk->text());
          });
}

bool DlgCreatePeer::check_machine() {
  ui->lbl_err_os->show();
  ui->pb_peer->setEnabled(true);
  ui->lbl_err_os->setStyleSheet("QLabel {color : green}");
  static size_t i;
  for (i = 0; i < m_requirements_ls.size(); i++) {
    ui->pb_peer->setValue(i + 1);
    ui->lbl_err_os->setText(m_requirements_ls[i].status_label);
    if (!m_requirements_ls[i].checker_function()) {
      ui->lbl_err_os->setText(m_requirements_ls[i].error_label);
      CNotificationObserver::Error(m_requirements_ls[i].error_notification,
                                   m_requirements_ls[i].notification_type);
      break;
    }
  }
  return i == m_requirements_ls.size();
}

void DlgCreatePeer::set_enabled_buttons(bool state) {
  ui->le_disk->setEnabled(state);
  ui->le_name->setEnabled(state);
  ui->le_pass->setEnabled(state);
  ui->le_pass_confirm->setEnabled(state);
  ui->le_ram->setEnabled(state);
  ui->btn_create->setEnabled(state);
  ui->btn_cancel->setEnabled(state);
}

void DlgCreatePeer::hide_err_labels() {
  ui->lbl_err_cpu->hide();
  ui->lbl_err_disk->hide();
  ui->lbl_err_ifs->hide();
  ui->lbl_err_name->hide();
  ui->lbl_err_pass->hide();
  ui->lbl_err_ram->hide();
  ui->lbl_err_os->hide();
}

// for peers, empty if that peer dir exists
QString DlgCreatePeer::create_dir(const QString &name) {
  QString new_dir = "";
  QDir current_local_dir;
  QStringList stdDirList =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  QStringList::iterator stdDir = stdDirList.begin();
  if (stdDir == stdDirList.end())
    current_local_dir.setCurrent("/");
  else
    current_local_dir.setCurrent(*stdDir);
  current_local_dir.cd("Subutai-peers");
  if (!current_local_dir.mkdir(name)) return new_dir;
  current_local_dir.cd(name);
  return current_local_dir.absolutePath();
}

void DlgCreatePeer::init_completed(system_call_wrapper_error_t res, QString dir,
                                   QString ram, QString cpu, QString disk) {
  if (res != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error(
        tr("Coudn't create peer, sorry. Check if all software is installed "
           "correctly"),
        DlgNotification::N_NO_ACTION);
    ui->btn_create->setEnabled(true);
    set_enabled_buttons(true);
    ui->pb_peer->setValue(0);
    ui->pb_peer->setEnabled(false);
    ui->lbl_err_os->setText(tr("Failed to initialize environment"));
    ui->lbl_err_name->setStyleSheet("QLabel {color : red}");
    QDir dir(dir);
    dir.removeRecursively();
    return;
  }
  CNotificationObserver::Instance()->Info(
      tr("Initialization is completed. Installing peer... Don't close terminal "
         "until installation is finished."),
      DlgNotification::N_NO_ACTION);
  QString filename = QString("%1/vagrant-subutai.yml").arg(dir);
  QFile file(filename);
  // write config file
  if (file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&file);
    stream << "SUBUTAI_RAM : " << ram << endl;
    stream << "SUBUTAI_CPU : " << cpu << endl;
    QString branch = current_branch_name();
    if (branch == "production")
      stream << "SUBUTAI_ENV : "
             << "prod" << endl;
    else if (branch == "development")
      stream << "SUBUTAI_ENV : "
             << "dev" << endl;
    else
      stream << "SUBUTAI_ENV : "
             << "master" << endl;
    stream << "DISK_SIZE : " << disk << endl;
    stream << "BRIDGE : "
           << QString("\"%1\"").arg(this->ui->cmb_bridge->currentText())
           << endl;
  }
  file.close();
  // write provision step file
  QDir pr_dir;
  pr_dir.mkdir(dir + QDir::separator() + ".vagrant");
  QString p_name = dir + QDir::separator() + ".vagrant" + QDir::separator() +
                   "provision_step";
  QFile p_file(p_name);
  if (p_file.open(QIODevice::ReadWrite)) {
    QTextStream stream(&p_file);
    stream << "0" << endl;
  }
  p_file.close();
  static QString vagrant_up_string = "up --provider virtualbox";
  QString peer_name = ui->le_name->text(), peer_pass = ui->le_pass->text();
  CSettingsManager::Instance().set_peer_pass(peer_name, peer_pass);
  res = CSystemCallWrapper::vagrant_command_terminal(dir, vagrant_up_string,
                                                     ui->le_name->text());
  if (res != SCWE_SUCCESS) {
    CNotificationObserver::Instance()->Error("Coudn't start  peer, sorry",
                                             DlgNotification::N_NO_ACTION);
    set_enabled_buttons(true);
  } else
    this->close();
}
