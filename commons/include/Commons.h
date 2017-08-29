#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>

#define UNUSED_ARG(x) ((void)x)


////////////////////////////////////////////////////////////////////////////

class CCommons {
public:
  static const char* RESTARTED_ARG;

  static char* CurrentDateFileNameString(void);

  static char* CurrentDateTimeString(void);

  static bool QuitAppFlag;

  static QString AppNameTmp(void);

  static QString FileMd5(const QString& file_path);

  static QString NetworkErrorToString(int err_code);

  static void RestartTray();
};

#endif // COMMONS_H
