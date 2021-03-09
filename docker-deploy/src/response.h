#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include "headers.h"
#include "myexception.h"
class Response{
public:
  std::string buf;  //all message for the response
  std::string statusCode;
  //std::string matchedReqHead;
  std::string matchedReqUrl;
  std::string resHeaderLine;
  std::string lastModified;
  std::string etag;
  
  int cacheType = 0;
  std::time_t birthTime = -1;  //the birth time of this response
  int age = 0;
  std::time_t expireTime = -1;
   
  std::map<std::string, std::string> resBody;
  Response();
  Response(std::string resAll, std::string reqHeader);
  ~Response();
  void setHeaderLine();  
  std::string getHeaderLine();
  void setStatusCode();
  std::string getStatusCode();
  //void setBuf();
  std::string getBuf();
  void setResBody();
  std::map<std::string, std::string> getResbody();
  
  //void setMatchedReqHead(std::string reqHead);
  std::string getMatchedReqUrl();
  void setCacheValue(int cacheTypeNum);
  std::string bodyFind(std::string item);
  int ageStrtoAge(std::string ageStr);
  std::time_t timeStr_to_timeT(std::string time);
  std::string getLastModified();
  std::string getEtag();
};
#endif
