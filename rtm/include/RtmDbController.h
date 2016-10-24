#ifndef RTMDBCONTROLLER_H
#define RTMDBCONTROLLER_H

#include <stdint.h>
#include <QtSql/QSqlDatabase>

namespace rtm {

  class CRtmDbController {
  private:

    enum err_codes {
      RDCE_SUCCESS = 0,
      RDCE_NO_DB_DRIVER,
      RDCE_DB_OPEN,
      RDCE_CANT_ADD_DB
    };

    CRtmDbController();
    ~CRtmDbController();
    bool create_tables();

    int32_t m_last_error;
    QSqlDatabase m_db;

    /*copy and assignment are prohibited*/
    CRtmDbController(const CRtmDbController&);
    CRtmDbController& operator=(const CRtmDbController&);

    static void log_sql_error(const QSqlError& err);

  public:

    static CRtmDbController& Instance() {
      static CRtmDbController instance;
      return instance;
    }

    bool is_valid() const {return m_last_error == 0;}
    QString& error_msg() const;
  };

}
#endif // RTMDBCONTROLLER_H
