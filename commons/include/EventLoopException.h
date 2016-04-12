#ifndef EVENTLOOPEXCEPTION_H
#define EVENTLOOPEXCEPTION_H
#include <string>
#include <exception>

class CEventLoopException : public std::exception
{
private:
  std::string m_Message;
public:
  CEventLoopException(void){}
  CEventLoopException(std::string msg) : m_Message(msg){}
  virtual ~CEventLoopException(void) throw(){}
  virtual const char* what() const throw(){return m_Message.c_str();}
};


#endif // EVENTLOOPEXCEPTION_H
