#include "socket.h"
#include "proxy.h"
#include "headers.h"
#include "myexception.h"
#include "response.h"
#include "cache.h"
//using namespace std;

//parse the clientFd to string
std::string parseFdtoString(int clientFd){
  char buf[512];
  int byte_count;
  if( byte_count = recv(clientFd, buf, sizeof(buf), 0) == -1 ){
    throw ErrorException("receive fail");
  }
  std::string bufString = buf;
  return bufString;
}

int main(){
  Socket mySocket;
  mySocket.sockfd = -1;
  mySocket.node = NULL;
  mySocket.service = "12345";
  
  try{
    mySocket.setup();
    std::cout<< "success setup" << std::endl;
  }
  catch(ErrorException &e){
    std::cout<< e.what() << std::endl;
    return EXIT_FAILURE;
  }
  
  try{
    mySocket.myListen();
    std::cout << "ready to listen" << std::endl;
  }
  catch(ErrorException &e){
    std::cout<< e.what() << std::endl;
    return EXIT_FAILURE;
  }

//  Proxy *myProxy = new Proxy(mySocket);
  Proxy myProxy;
  myProxy.sock = mySocket;
  //Request req = myProxy.getRequest();
  //myProxy.parseRequest(req);
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++test Cache
  /*
  Response res1(req.getAllMessage(), req.getHeaderLine());
  //std::cout << "res1 h" << res1.getHeaderLine() << std::endl;
  //std::cout << "res1 code" << res1.getStatusCode() << std::endl;
  //std::cout << "res1 body" << res1.getResBody() << std::endl;
  Response res2(req.getAllMessage(), req.getHeaderLine());
  Response res3(req.getAllMessage(), req.getHeaderLine());
  //res.buf += "***";
  //std::cout << "res is : " << std::endl << res2.getBuf() << std::endl; 
  Cache testCache;
  //std::string testHeader1 = req.getHeaderLine();
  //std::string testHeader2 = "1" + req.getHeaderLine();
  testCache.add(&res1);
  testCache.add(&res2);
  testCache.add(&res3);
  std::map<std::string, Response*> testResponseMap = testCache.responseMap;
  std::map<std::string, Response*>::iterator testMapIt;  //map iterator
  std::list<std::string> testCacheList = testCache.cacheList;
  std::list<std::string>::iterator testListIt;  //list iterator
  for(testMapIt = testResponseMap.begin(); testMapIt != testResponseMap.end(); testMapIt ++){
    std::cout << testMapIt->first << "  map " << std::endl;
  }
  for(testListIt = testCacheList.begin(); testListIt != testCacheList.end(); testListIt ++){
    std::cout << *testListIt << "  list " << std::endl;
  }
  std::cout <<"hahaha  " << testCache.find(req.getHeaderLine()) -> resHeaderLine << std::endl;*/
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++test cache
  Request request = myProxy.getRequest();
  myProxy.parseRequest(request);
  std::string testResStr = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nX-Cloud-Trace-Context: a5d237eac4db95e0d3850364546e1d2b\r\nDate: Wed, 03 Mar 2021 09:15:51 GMT\r\nServer: Google Frontend\r\nContent-Length: 17\r\n\r\nhtml code\r\n";
  //std::cout << testResStr << std::endl;
  Response res(testResStr, request.getHeaderLine());
  std::cout << "res head: " << res.getHeaderLine() << std::endl;
  std::cout << "res code: " << res.getStatusCode() << std::endl;
  std::cout << "res req head: " << res.getMatchedReqHead() << std::endl;
  std::cout << "res all: " << res.getBuf() << std::endl;
    //std::string matchedReqHead;
   //_______________________________________________________________________________________________________________________________________test Cache 
  Cache singleCache;
  singleCache.add(&res);

if(method == "GET"){  
  //variable: singleCache, response
  std::string reqHeader = request.getHeaderLine();
  *Response resInCache = singleCache.find();
  if(resInCache == NULL){  //cache miss
    //get a response from server
    Response res = recv();  //new res
    std::string resAllMessage = res.getBuf();
    //when response is enpty
    if(resAllMessage.length() == 0){
      return;
    }
    std::string statusCode = res.getStatusCode();
    if(statusCode== "200"){  //response success
      //待会写判断是否缓存
      int cacheType = cacheType(res);
      if(cacheType == 1 || cacheType == 2){  //一定有cache control字段
        res.setCacheValue(cacheType);
        cache.add(&res);
      }
      //senf back to client
    }
    else if(statusCode == "304"){
      //log: print 服务器已经缓存了请求，此时不用cache，但也不是error
      //如何显示为改变的前一个request？待解决
    }
    else if(statusCode[0] == '3'){
      //log报错
    }
    else if(statusCode[0] == '4'){
      //log 报错
    }
    else if(statusCode[0] == '5'){
      //log报错
    }
    else{
      //log 未知错误
    }    
    //在miss时，无论什么情况都要将服务器发来的response发送给client
    //待写：~~~~~~
  }
  else{  //hit
    std::time_t currentTime = std::time(nullptr); 
    if(currentTime < res.expireTime){  //still fresh
       updateToMostUse(&resInCache);  //update      
    }
    else{  //expired
      
    }
  }  
}

//identify the cache type of the response, i is the request id, use for write log, add later
//std::string cacheType(Response res, int i){
//0 for no_store, 1 for must revalidate, 2 for validation after expire
int cacheType(Response res){
  std::map<std::string, std::string> resBody = res.getResbody(); 
   std::string pragma = resBodyFind(resBody, "pragma");
  std::string cacheControl = resBodyFind(resBody, "Cache-Control");
  //std::string eTag = resBodyFind(resBody, "ETag");
  if(pragma.find("no-cache") != std::string::npos){
    return 1;
  } 
  if(CacheControl.length() == 0){
    return 0;  //no store
  }
  if(CacheControl.find("no-store") != std::string::npos || cacheControl.find("private") != std::string::npos){
    return 0;
  }
  if(CacheControl.find("age") != std::string::npos || CacheControl.find("revalidate") != std::string::npos || CacheControl.find("public") != std::string::npos){
    return 2;  //if expire, revalidate 
  }
  if(CacheControl.find("no-cache") != std::string::npos){
    return 1;
  }
  return 0;
}

//find the comment of the specific title in resBodyFind
std::string resBodyFind(std::map<std::string, std::string> resBody, std::string item){
  std::map<std::string, std::string>::iterator it;
  it = resBody.find(item);
  if(it != resBody.end()){
    return it->second;
  }
  return "";
}
  
  
  
  
  
 //_______________________________________________________________________________________________________________________________________test Cache  
  
  
  
  
  
  
  
  

  
  
  
  
  
  
  
  
  
  
  //myProxy.processRequest(req);
//  std::cout<< close(req.clientFd) << std::endl;
//  std::cout<< close(myProxy.sock.sockfd) << std::endl;
/*    
  int clientFd;
  while(true){
    try{
      std::cout << "before accept" << std::endl;
      std::cout << mySocket.sockfd << std::endl;
      clientFd = mySocket.myAccept();
      std::cout << "after accept" << std::endl;
      break;
    }
    catch(ErrorException &e){
      std::cout<< e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;*/      
}