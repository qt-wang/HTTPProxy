#ifndef __WRITELOG_H__
#define __WRITELOG_H__
#include "headers.h"
#include "myexception.h"
class writeLog{
public:
  writeLog();
  ~writeLog();
  
  void writeInLog(std::string logComment);
};



#endif