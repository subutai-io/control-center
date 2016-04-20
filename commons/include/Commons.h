#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>

class CCommons {
public:
  static char* CurrentDateFileNameString(void);
  static char* CurrentDateTimeString(void);
  static bool QuitAppFlag;
};

#endif // COMMONS_H
