#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>

#define UNUSED_ARG(x) ((void)x)

//don't change order of this enum. used by Makefile
enum branch_t {
  BT_MASTER = 0,
  BT_DEV
};

//don't change order of this enum. used by Makefile
enum os_t {
  OS_LINUX = 0,
  OS_WIN,
  OS_MAC
};
////////////////////////////////////////////////////////////////////////////

/**
 * @brief Methods, macroses, structures, constants and other fields of this class could be used in any part of this program
 */
class CCommons {
public:
  /*!
   * \brief RESTARTED_ARG - constant string that is using by RestartTray method and in main.cpp for checking
   * if tray was restarted.
   */
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

  /*!
   * \brief NetworkErrorToString - QtNetworkError code to string.
   * \param err_code - if QtNetworkError list doesn't contain this err_code method will return UndefinedError.
   * \return
   */
  static QString NetworkErrorToString(int err_code);

  /*!
   * \brief Restart tray application with added RESTARTED_ARG
   */
  static void RestartTray();
};

#endif // COMMONS_H
