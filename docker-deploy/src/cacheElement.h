#ifndef __CACHEELEMENT_H__
#define __CACHEELEMENT_H__
#include "headers.h"
#include "myexception.h"

class cacheElement{
public:
  std::string getMark;  //the first line of the corresponding request
  Response res;    //the response itself
  std::time_t initialTime;  //the time when it stored in cache
  std::
  
  
  //std::map<std::string, std::string> eleBody; //store the body message for a chche element
  
  

};

#endif