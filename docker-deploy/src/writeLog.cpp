#include "writeLog.h"
#define LOG_PATH 
writeLog::writeLog(){}
writeLog::~writeLog(){}
  
void writeLog::writeInLog(std::string logComment){
    std::ofstream out;
    out.open("/var/log/erss/proxy.log", std::ios::app);
    out << logComment;
    out.close();
}