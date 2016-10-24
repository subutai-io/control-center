#ifndef FILEWRAPPER_H
#define FILEWRAPPER_H

#include <stdio.h>

/*!
 * \brief The CFileWrapper class encapsulates file operations.
 * \brief Use stack objects of this class for file operations.
 */
class CFileWrapper {
private:
  FILE *m_file;
  CFileWrapper(void);
  CFileWrapper(const CFileWrapper&);
  void operator=(const CFileWrapper&);

public:
  /*!
   * \brief CFileWrapper constructor tries to open file.
   * \param fileName - full path to file
   * \param flags - like in fopen function.
   */
  CFileWrapper(const char* fileName, const char* flags) { m_file = fopen(fileName, flags);}

  /*!
    * \brief ~CFileWrapper destructor closes file
    */
  ~CFileWrapper(void) { if (m_file != NULL) fclose(m_file);}

  /*!
   * \brief FRead - calls fread function
   */
  size_t FRead(void* buff, size_t elemSize, size_t count) { return fread(buff, elemSize, count, m_file);}

  /*!
   * \brief FWrite - calls fwrite function
   */
  size_t FWrite(void* str, size_t size, size_t count) {return fwrite(str, size, count, m_file);}

  /*!
   * \brief FSeek - calls fseek function
   */
  int FSeek(long offset, int origin) {return fseek(m_file, offset, origin);}

  /*!
   * \brief FPrintf - use it like printf function
   */
  int FPrintf (const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf(m_file, format, args);
    va_end(args);
    return result;
  }

  /*!
   * \brief shows that file was opened and everything is fine.
   */
  bool IsValid(void) const {return m_file != NULL;}
};

#endif // FILEWRAPPER_H
