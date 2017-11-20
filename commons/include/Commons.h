#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>
#include <QStringList>

#define UNUSED_ARG(x) ((void)x)


////////////////////////////////////////////////////////////////////////////

class CCommons {
public:
  static const char* RESTARTED_ARG;

  static QString FileMd5(const QString& file_path);
  static QString NetworkErrorToString(int err_code);
  static void RestartTray();
  static bool IsApplicationLaunchable(const QString& file_path);
  static bool IsTerminalLaunchable(const QString& terminal);

  static const QString TERMINAL_WRONG_ARG;
  static bool HasRecommendedTerminalArg(const QString& terminalCmd,
                                     QString &recommendedArg);
  static QStringList DefaultTerminals();
  static QStringList SupportTerminals();
};

#endif // COMMONS_H
