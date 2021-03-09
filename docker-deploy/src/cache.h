#ifndef __CACHE_H__
#define __CACHE_H__
#include "headers.h"
#include "myexception.h"
#include "response.h"
#include "request.h"

class Cache{
public:
  //size = 1000
  int size;
  //first is the corresponding request url, second is response ptr
  std::map<std::string, Response*> responseMap;
  //store the response's corresponding request's url
  std::list<std::string> cacheList;
  
  Cache();
  ~Cache();
    
  //with a given request head, find if the corresponding response is in cache
  //true for find it, and move it to the head of the list
  //false for not found
  Response* find(std::string requestUrl);
  
  //remove the least used cache
  void remove();
  
  //add the new cacheElement to the cache
  void add(Response *res);
  
  void updateToMostUse(Response *res);
};


#endif