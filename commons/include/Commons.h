#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>
#include <QStringList>

#define UNUSED_ARG(x) ((void)x)


////////////////////////////////////////////////////////////////////////////

/**
 * @brief The CCommons contains common methods and constants
 */
class CCommons {
public:
  static const char* RESTARTED_ARG;

  /**
   * @brief MD5 hash of file (like md5sum)
   * @param file_path - path to file
   * @return MD5 hash of file's content as QString
   */
  static QString FileMd5(const QString& file_path);

  /**
   * @brief Used only in login dialog.
   * @param QNetworkReply::NetworkError err_code
   * @return QNetworkReply::NetworkError as QString or "Unknown error"
   */
  static QString NetworkErrorToString(int err_code);

  /**
   * @brief Exit current process and start SubutaiTray application with RESTARTED_ARG argument
   */
  static void RestartTray();

  /**
   * @brief Check if application is launchable
   * @param file_path - path to application
   * @return true if application exists and has execute permission, otherwise - false
   */
  static bool IsApplicationLaunchable(const QString& file_path);
  static bool IsTerminalLaunchable(const QString& terminal);
  static bool IsVagrantVMwareLicenseInstalled();

  /**
   * @brief If there is recommended argument for terminal "terminalCmd" in table
   * returns true and initializes "recommendedArg"
   */
  static bool HasRecommendedTerminalArg(const QString& terminalCmd,
                                     QString &recommendedArg);

  /**
   * @brief Returns terminals which have recommended arguments
   */
  static QStringList DefaultTerminals();
  static QStringList SupportTerminals();
  static QString GetFingerprintFromUid(const QString &uid);
  static QString HomePath();

};

#endif // COMMONS_H
