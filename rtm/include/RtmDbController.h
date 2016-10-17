#ifndef RTMDBCONTROLLER_H
#define RTMDBCONTROLLER_H

#include <stdint.h>
#include <QtSql/QSqlDatabase>

namespace rtm {

  class CRtmDbController {
  private:
    CRtmDbController();
    ~CRtmDbController();
    bool create_tables();

    int32_t m_last_error;
    QSqlDatabase m_db;

    /*copy and assignment are prohibited*/
    CRtmDbController(const CRtmDbController&);
    CRtmDbController& operator=(const CRtmDbController&);

  public:

    CRtmDbController& Instance() {
      static CRtmDbController instance;
      return instance;
    }

    bool is_valid() const {return m_last_error == 0;}
  };

}
#endif // RTMDBCONTROLLER_H
