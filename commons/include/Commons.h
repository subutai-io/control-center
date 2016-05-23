#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>

class CCommons {
public:

  /*!
   * \brief CurrentDateFileNameString
   * \return current_date.txt string
   */
  static char* CurrentDateFileNameString(void);

  /*!
   * \brief CurrentDateTimeString
   * \return current_date_time.txt string
   */
  static char* CurrentDateTimeString(void);

  /*!
   * \brief QuitAppFlag - flag to determine that Application is about to quit.
   */
  static bool QuitAppFlag;

  /*!
   * \brief AppNameTmp - temporary file name.
   * \return SubutaiTray.tmp
   */
  static QString AppNameTmp(void);
};

#endif // COMMONS_H
