#include "socket.h"
#include "proxy.h"
#include "headers.h"
#include "myexception.h"
//using namespace std;

//parse the clientFd to string
std::string parseFdtoString(int clientFd){
  char buf[512];
  int byte_count;
  if( byte_count = recv(clientFd, buf, sizeof(buf), 0) == -1 ){
    throw myException("receive fail");
  }
  std::string bufString = buf;
  return bufString;
}

int main(){

/*******************multi-thread*****************************/ 
  Proxy myProxy(1, 12345);
  try{
    myProxy.beginService();
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
/************************************************/  
     
}