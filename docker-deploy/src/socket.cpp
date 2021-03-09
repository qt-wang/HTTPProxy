#include "myexception.h"
#include "socket.h"


Socket::Socket(): sockfd(0), node(NULL), service(NULL), res(NULL){}

Socket::~Socket(){} 

void Socket::setup(){
  int status;
  memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
  hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  if(status = getaddrinfo(node, service, &hints, &res) != 0){
    throw myException("getaddrinfo should be 0, but is " + status);
  }
  
  //use socket()
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(sockfd == -1){
    throw myException("socket should be 0, but is " + sockfd);
  }
  int on = 1;
  int ret;
  ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  //use bind()
//  int b = bind(sockfd, res->ai_addr, res->ai_addrlen);
  ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
  std::cout << "bind num " << ret <<std::endl; 
  if(ret != 0){
  //if(bind(sockfd, res->ai_addr, res->ai_addrlen) != 0){
    throw myException("bind fail");
  }
}

void Socket::myListen(){
  //use listen()
  int l = listen(sockfd, 200);
  std::cout << "listen num " <<  l<<std::endl; 
  if(l!= 0){
  //if(listen(sockfd, 200) != 0){
    throw myException("listen fail");
  }
}

int Socket::myAccept(sockaddr_in &their_addr, socklen_t &addr_size){
//  struct sockaddr_storage their_addr;
//  socklen_t addr_size = sizeof(their_addr);
  int new_fd;
  //std::cout << "new_fd"  << std::endl;
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
  //std::cout << "new_fd2" << new_fd  << std::endl;
  if(new_fd == -1){
    throw myException("accept fail");
  }
  return new_fd;
}
