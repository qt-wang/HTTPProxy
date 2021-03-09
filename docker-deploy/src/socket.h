#ifndef _SOCKET_H
#define _SOCKET_H
#include "headers.h"
#include "request.h"
class Socket{
public:
  int sockfd;
  const char * node;  //IP
  const char * service;  //port number
  struct addrinfo hints;
  struct addrinfo *res;
  Socket();
  ~Socket();
  void setup();
  void myListen();
  int myAccept(sockaddr_in &their_addr, socklen_t &addr_size);
 // int serverInitSocket(const std::string &host, const std::string &port, Request &req);
};

#endif
  