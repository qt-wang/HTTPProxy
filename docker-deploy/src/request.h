#ifndef __REQUEST_H__
#define __REQUEST_H__
#include "headers.h"
#include "myexception.h"
class Request{
public:
  int reqId;
  int clientFd;
  std::string clientIp;
  std::string time;
  int byte_num;
  std::string allMessage;
  std::string headerLine;
  std::string reqUrl;
  std::string method;
  std::string host;
  std::string port;
  std::string requestTime;
  //std::map<std::string, std::string> header;
  std::map<std::string, std::string> reqBody;
    
  Request(int clientSockfd, std::string ip, std::string time);
  Request(int clientSockfd, std::string ip, std::string time, int id_for_req);
  ~Request();
  //set all message, headerline and method, host, port, reqBody
  void setUp(std::string message, std::string time);
  
  void setHeaderLine(std::string message);
  std::string getHeaderLine();

  void setUrl(std::string message);
  std::string getUrl();

  void setMethod(std::string headerLine);
  std::string getMethod();
  
  void setBody(std::string message);
  
  void setAllMessage(std::string message);
  std::string getAllMessage();
  
  std::string getHostComment();
  void setHost();
  std::string getHost();
  
  void setPort();
  std::string getPort(); 
  void setRequestTime(std::string time);
  std::string getRequestTime(); 
  void addToBody(std::string title, std::string comment);
};

#endif