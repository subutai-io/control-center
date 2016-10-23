#include <QSqlError>
#include <QStringList>
#include <QDir>
#include <QSqlQuery>

#include "rtm/include/RtmDbController.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"

using namespace rtm;

static const char* DB_DRIVER = "QSQLITE";
static const char* DB_NAME = "rtm.sqlite";

QString rtm_db_name() {
  return CSettingsManager::Instance().rtm_db_dir() +
      QDir::separator() + DB_NAME;
}

////////////////////////////////////////////////////////////////////////////

CRtmDbController::CRtmDbController() :
  m_last_error(RDCE_SUCCESS) {
  QString db_file_path = rtm_db_name();

  do {
    QSqlDatabase::database(db_file_path, false).close();
    QSqlDatabase::removeDatabase(db_file_path);

    if (!QSqlDatabase::drivers().contains(DB_DRIVER)) {
      m_last_error = RDCE_NO_DB_DRIVER;
      break;
    }

    m_db = QSqlDatabase::addDatabase(DB_DRIVER, db_file_path);
    if (!m_db.isValid()) {
      m_last_error = RDCE_CANT_ADD_DB;
      QSqlError err = m_db.lastError();
      log_sql_error(err);
      break;
    }

    m_db.setDatabaseName(db_file_path);

    if (!m_db.open()) {
      m_last_error = RDCE_DB_OPEN;
      QSqlError err = m_db.lastError();
      log_sql_error(err);
      break;
    }

    create_tables();

  } while (0);
}
////////////////////////////////////////////////////////////////////////////

CRtmDbController::~CRtmDbController() {
  m_db.close();
}
////////////////////////////////////////////////////////////////////////////

bool
CRtmDbController::create_tables() {
  static const char* commands[] = {
    "create table if not exists rh_sys_info (id integer primary key, Date NUMERIC, Data BLOB)",
    "create index if not exists ix_rh_sys_info_date ON rh_sys_info(Date ASC)",
    nullptr
  };

  QSqlQuery q("", m_db);
  const char** tmp = commands;
  for (;*tmp!=nullptr;++tmp) {
    if (!q.exec(*tmp)) {
      CApplicationLog::Instance()->LogError("Query \"%s\" failed", *tmp);
      log_sql_error(q.lastError());
      continue;
    }
  }

  return false;
}
////////////////////////////////////////////////////////////////////////////

void
CRtmDbController::log_sql_error(const QSqlError &err) {
  CApplicationLog::Instance()->LogError("%s",
                                        err.databaseText().toStdString().c_str());
  CApplicationLog::Instance()->LogError("%s",
                                        err.driverText().toStdString().c_str());
  CApplicationLog::Instance()->LogError("%s",
                                        err.nativeErrorCode().toStdString().c_str());
}
////////////////////////////////////////////////////////////////////////////

QString&
CRtmDbController::error_msg() const {
  static QString error_messages[] = {
    "Success",
    "QSQLite driver isn't presented in system",
    "Can't open data base"
  };
  return error_messages[m_last_error];
}
////////////////////////////////////////////////////////////////////////////
