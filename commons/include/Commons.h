#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>

#define UNUSED_ARG(x) ((void)x)

class CCommons {
public:
  static const char* RESTARTED_ARG;

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

  /*!
   * \brief FileMd5
   * \param file_path
   * \return
   */
  static QString FileMd5(const QString& file_path);

  static QString NetworkErrorToString(int err_code);

  static void RestartTray();
};

#endif // COMMONS_H
