#include "Commons.h"
#include <time.h>

static const char* date_format = "%d.%m.%Y.txt";
static char date_str_buffer[15];
char *CCommons::CurrentDateFileNameString() {
  time_t ct = time(NULL); //system now
  tm now;
#ifndef RT_OS_WINDOWS
  localtime_r(&ct, &now);
#else
  localtime_s(&now, &ct);
#endif
  strftime(date_str_buffer, 15, date_format, &now);
  return date_str_buffer;
}
////////////////////////////////////////////////////////////////////////////

static const char* date_time_format = "%d.%m.%Y %H:%M:%S";
static char date_time_str_buffer[20];
char *CCommons::CurrentDateTimeString() {
  time_t ct = time(NULL); //system now
  tm now;
#ifndef RT_OS_WINDOWS
  localtime_r(&ct, &now);
#else
  localtime_s(&now, &ct);
#endif
  strftime(date_time_str_buffer, 20, date_time_format, &now);
  return date_time_str_buffer;
}
////////////////////////////////////////////////////////////////////////////

