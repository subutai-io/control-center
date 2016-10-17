#include "rtm/include/RtmDbController.h"

using namespace rtm;

CRtmDbController::CRtmDbController() :
  m_last_error(0) {

  static const char* db_name = "rtm_db";

  QSqlDatabase::database(db_name, false).close();
  QSqlDatabase::removeDatabase(db_name);

  m_db = QSqlDatabase::addDatabase("SQLITE", db_name);
}

CRtmDbController::~CRtmDbController() {

}
////////////////////////////////////////////////////////////////////////////

bool
CRtmDbController::create_tables() {
  static const char* commands[] = {
    "create table if not exists rh_sys_info (id integer primary key, Date NUMERIC, Data BLOB)",
    "create index if not exists ix_rh_sys_info_date ON rh_sys_info(Date ASC)",
    nullptr
  };

  const char** tmp = commands;
  for (;*tmp!=nullptr /*&& execute_command by qtsqlquery*/;++tmp) {
    //todo execute commands
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////
