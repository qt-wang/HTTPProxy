#ifndef __PROXY_H__
#define __PROXY_H__
#include "headers.h"
#include "request.h"
#include "socket.h"
#include "myexception.h"
#include "response.h"
#include "cache.h"
#include "writeLog.h"

class Proxy{
public:
  int requestId;
  int port;
//  Cache *cache;
  Socket sock;
  Cache singleCache;
  writeLog myLog;
    
  Proxy(int id, int portNum);
  ~Proxy();
  Request *getRequest(int requestIdNum);
  void parseRequest(Request *req);
  Response communicateServer(Request *req);
  Response receiveResponse(int sockfd, std::string reqHeaderLine);
  void sendResponseToClient(Response res, int clientFd);
  void processRequest(Request *req);
  int serverInitSocket(const std::string &host, const std::string &port, Request *req);
  int setCacheType(Response res);
  std::string resBodyFind(std::map<std::string, std::string> resBody, std::string item);
  void serverConnect(Request *req);
  void beginService();
};

#endif