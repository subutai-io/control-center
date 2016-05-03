#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>

class CCommons {
public:
  static char* CurrentDateFileNameString(void);
  static char* CurrentDateTimeString(void);
  static bool QuitAppFlag;
  static QString AppNameTmp(void);
};

#endif // COMMONS_H
