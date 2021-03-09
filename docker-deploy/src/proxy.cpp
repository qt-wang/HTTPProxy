#include "proxy.h"
#include "cache.h"
#include "myexception.h"
Proxy::Proxy(int id, int portNum):requestId(id), port(portNum){}

Proxy::~Proxy(){}


void Proxy::beginService(){
    Socket mySocket;
    mySocket.sockfd = -1;
    mySocket.node = NULL;
    mySocket.service = "12345";
    this->sock = mySocket;
    try{
      this->sock.setup();
      std::cout<< "success setup" << std::endl;
    }
    catch(myException &e){
      std::cout<< e.what() << std::endl;
    }
    
    try{
      this->sock.myListen();
      std::cout << "ready to listen" << std::endl;
    }
    catch(myException &e){
      std::cout<< e.what() << std::endl;
    }   
    while(true){
      Request *req = this->getRequest(requestId);
      this->parseRequest(req);
        requestId ++;
      std::cout << req->getHeaderLine() << std::endl;
      std::thread t(&Proxy::processRequest, this , req);
      t.detach();
    }
}

Request *Proxy::getRequest(int requestIdNum){
  int clientFd;
  struct sockaddr_in their_addr; 
  memset(&their_addr, 0, sizeof (their_addr));
  socklen_t addr_size = sizeof(their_addr);
//  while(true){
    try{
       clientFd = sock.myAccept(their_addr, addr_size);     
      std::time_t seconds = std::time(NULL);
        std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
        request_time = request_time.substr(0, request_time.find("\n"));

        char client_ip_addr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(their_addr.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL) {
            throw myException("inet_ntop() failed");
        }
        std::string client_ip = std::string(client_ip_addr);
        Request *client_request = new Request(clientFd, client_ip, request_time, requestIdNum);
        return client_request;        
    }
    catch(myException &e){
      std::cout<< e.what() << std::endl;
      exit(EXIT_FAILURE);
    }
//  }
}

void Proxy::parseRequest(Request *req){
  //receive()
  int byte_count;
  char buf[4096];
  memset(buf, 0, sizeof(buf));
  byte_count = recv(req->clientFd, buf, 4095, 0);
  buf[byte_count] = '\0';
  if(byte_count == 0){
    return;
  }
  std::string request_message;
  std::stringstream stream;
  stream << buf;
  request_message = stream.str();
  try{
    std::time_t requestTime = std::time(nullptr);
    std::string requestTimeSrtPre = std::string(std::asctime(std::gmtime(&requestTime)));
    std::string requestTimeSrt = requestTimeSrtPre.substr(0, requestTimeSrtPre.find("\n"));
    req->setUp(request_message, requestTimeSrt);
    std::string reqSetUpLog = std::to_string(req->reqId) + ": \"" + req->getHeaderLine() + "\" from " + req->clientIp + " @ " + req->getRequestTime() + "\n";
    myLog.writeInLog(reqSetUpLog);
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
}

void Proxy::processRequest(Request *req){ 
    // POST method
    if(req->getMethod() == "POST"){
      std::string reqSetUpLog = std::to_string(req->reqId) + ": Requesting \"" + req->getHeaderLine() + "\" from " + req->reqUrl + "\n";
      myLog.writeInLog(reqSetUpLog);
      Response res;
      try{
        res = communicateServer(req);
      }
      catch(myException &e){
        std::cout<< e.what() << std::endl;
      }
      std::string postStatusCode = res.getStatusCode();
      if(postStatusCode == "200"){
        std::string resInPOST = std::to_string(req->reqId) + ": Received \"" + res.getHeaderLine() + "\" from " + req->reqUrl + "\n";
        myLog.writeInLog(resInPOST);
        //std::string status = res.getStatusCode(res.buf);
        std::string sendToClientInPOST = std::to_string(req->reqId) + ": Responding \"" + res.getHeaderLine()  + "\"\n";
        myLog.writeInLog(sendToClientInPOST);
        sendResponseToClient(res, req->clientFd);
      }
      else if(postStatusCode[0] == '3'){
          std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(warning);
          sendResponseToClient(res, req->clientFd);
      }
      else if(postStatusCode[0] == '4'){
        std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }
      else if(postStatusCode[0] == '5'){
        std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }
      else{
        std::string error = std::to_string(req->reqId) + ": ERROR Unknown error\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }    
      if (close(req->clientFd) == -1) {
        perror("client socket close() failed");
      }
    
    }
    // CONNECT method
    //else if(req.getMethod() == "POST" && i == 1){
    else if(req->getMethod() == "CONNECT"){
      try{
        serverConnect(req);
      }
      catch(myException &e){
        std::cout<< e.what() << std::endl;
      }
    }
    //else{
    else if(req->getMethod() == "GET"){
      std::string reqUrl = req->getUrl();
      Response * resInCache = singleCache.find(reqUrl);
      if(resInCache == NULL){  //cache miss
        std::string reqSetUpLog = std::to_string(req->reqId) + ": not in cache\n";
        myLog.writeInLog(reqSetUpLog);
        //get a response from server
        std::string reqMiss = std::to_string(req->reqId) + ": Requesting \"" + req->getHeaderLine() + "\" from " + req->reqUrl + "\n";
        myLog.writeInLog(reqMiss);
        Response res;
        try{
          res = communicateServer(req);  //new res
        }
        catch(myException &e){
          std::cout<< e.what() << std::endl;
        }
        std::string resForReqMiss = std::to_string(req->reqId) + ": Received \"" + res.getHeaderLine() + "\" from " + req->reqUrl + "\n";
        myLog.writeInLog(resForReqMiss);
        //Response res(testResStr, reqUrl);
        std::string resAllMessage = res.getBuf();
        //when response is enpty
        if(resAllMessage.length() == 0){
          throw myException("The response's length is 0!");
        }
        std::string statusCode = res.getStatusCode();
        if(statusCode== "200"){  //response success
          int cacheType = setCacheType(res);
          if(cacheType == 1 || cacheType == 2){
            res.setCacheValue(cacheType);
            singleCache.add(&res);
            if(cacheType == 1){
              std::string cacheLog = std::to_string(req->reqId) + ": cached, but requires re-validation\n";
              myLog.writeInLog(cacheLog);
            }
            else{
              std::string expireTimePre = std::string(std::asctime(std::gmtime(&(res.expireTime))));
              std::string expireTime = expireTimePre.substr(0, expireTimePre.find("\n"));
              std::string cacheLog = std::to_string(req->reqId) + ": cached, expires at " + expireTime + "\n";
              myLog.writeInLog(cacheLog);
            }
          }
          else{  //cacheType = 0, no store
            std::string NoStoreLog = std::to_string(req->reqId) + ": not cacheable because it is a No_Store type\n";
            myLog.writeInLog(NoStoreLog);
          }
        std::string sendToClientInGETMiss = std::to_string(req->reqId) + ": Responding \"" + res.getHeaderLine()  + "\"\n";
        myLog.writeInLog(sendToClientInGETMiss);
        sendResponseToClient(res, req->clientFd);
        //std::cout << "send back" << std::endl;
        }
        else if(statusCode == "304"){
      sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '3'){
          std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(warning);
          sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '4'){
          std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '5'){
          std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }
        else{
          std::string error = std::to_string(req->reqId) + ": ERROR Unknown error\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }    
      }
      else{  //hit
        std::time_t currentTime = std::time(nullptr); 
        //reminder: resInCache is a pointer!!!!!
        if(currentTime < resInCache->expireTime){  //still fresh
          std::string reqSetUpLog = std::to_string(req->reqId) + ": in cache, valid\n";
          myLog.writeInLog(reqSetUpLog);
          singleCache.updateToMostUse(resInCache);  //update
          //return response 
          //std::cout<< "response in GET:" << resInCache->getBuf()<<std::endl;
          std::string sendToClientInGETStillValid = std::to_string(req->reqId) + ": Responding \"" + resInCache->getHeaderLine()  + "\"\n";
          myLog.writeInLog(sendToClientInGETStillValid);
          sendResponseToClient(*resInCache, req->clientFd);     
        }
        else{  //expired
          if(resInCache->age == 0){  //age = 0, type is 1, need validation everytime
            std::string reqSetUpLog = std::to_string(req->reqId) + ": in cache, requires validation\n";
            myLog.writeInLog(reqSetUpLog);
          }
          else{
            std::string expiredTimeSrtPre = std::string(std::asctime(std::gmtime(&(resInCache->expireTime))));
            std::string expiredTimeSrt = expiredTimeSrtPre.substr(0, expiredTimeSrtPre.find("\n"));
            std::string reqSetUpLog = std::to_string(req->reqId) + ": in cache, but expired at " + expiredTimeSrt + "\n";
            myLog.writeInLog(reqSetUpLog);
          }  
          std::string lastMInRes = resInCache->getLastModified();
          std::string etagInRes = resInCache->getEtag();
          req->addToBody("If-Modified-Since", lastMInRes);   //req: If-Modified-Since <---> res: Last-Modified
          req->addToBody("If-None-Match", etagInRes);      //req: If-None-Match <---> res: ETag
          }        
          //get new response
          std::string reqHitButExpired = std::to_string(req->reqId) + ": Requesting \"" + req->getHeaderLine() + "\" from " + req->reqUrl + "\n";
          myLog.writeInLog(reqHitButExpired);     
          Response res;
          try{
            res = communicateServer(req);  //new res
          }
          catch(myException &e){
            std::cout<< e.what() << std::endl;
          }
          std::string resForReqHitButExpired = std::to_string(req->reqId) + ": Received \"" + res.getHeaderLine() + "\" from " + req->reqUrl + "\n";
          myLog.writeInLog(resForReqHitButExpired);
          std::string resAllMessage = res.getBuf();
          //when response is enpty
          if(resAllMessage.length() == 0){
          //perror no message in resAllmessage
            //return;
            throw myException("The response's length is 0!");
          }
          std::string statusCode = res.getStatusCode();
          if(statusCode== "200"){  //response success
            int cacheType = setCacheType(res);
            if(cacheType == 1 || cacheType == 2){ 
              if(cacheType == 1){
                std::string cacheLog = std::to_string(req->reqId) + ": cached, but requires re-validation\n";
                myLog.writeInLog(cacheLog);
              }
              else{
                std::string expireTimePre = std::string(std::asctime(std::gmtime(&(res.expireTime))));
                std::string expireTime = expireTimePre.substr(0, expireTimePre.find("\n"));
                std::string cacheLog = std::to_string(req->reqId) + ": cached, expires at " + expireTime + "\n";
                myLog.writeInLog(cacheLog);
              }
              res.setCacheValue(cacheType);
              singleCache.add(&res);
            }
            else{ //cacheType = 0
              std::string NoStoreLog = std::to_string(req->reqId) + ": not cacheable because it is a No_Store type\n";
              myLog.writeInLog(NoStoreLog);
            }
        //senf back to client
          //std::cout<< "response in GET:" << res.getBuf()<<std::endl;
          std::string sendBackGETExpireRes = std::to_string(req->reqId) + ": Responding \"" + res.getHeaderLine()  + "\"\n";
          myLog.writeInLog(sendBackGETExpireRes);
          sendResponseToClient(res, req->clientFd);        
        }
        else if(statusCode == "304"){
            singleCache.updateToMostUse(resInCache);
          //send back to client
            //std::cout<< "response in GET:" << resInCache->getBuf()<<std::endl;
            std::string getNoneChangeBack = std::to_string(req->reqId) + ": Responding \"" + resInCache->getHeaderLine()  + "\"\n";
            myLog.writeInLog(getNoneChangeBack);
            sendResponseToClient(*resInCache, req->clientFd);
          
          }
          else if(statusCode[0] == '3'){
            std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(warning);
            sendResponseToClient(res, req->clientFd);
          }
          else if(statusCode[0] == '4'){
            std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(error);
            sendResponseToClient(res, req->clientFd);
          }
          else if(statusCode[0] == '5'){
            std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(error);
            sendResponseToClient(res, req->clientFd);
          }
          else{
            std::string error = std::to_string(req->reqId) + ": ERROR Unknown error\n";
            myLog.writeInLog(error);
            sendResponseToClient(res, req->clientFd);
          }    
        }
      }
      else{
        std::string noMethod = ("HTTP/1.1 t found no source exist\r\n\r\n");
        std::string noMethodLog = std::to_string(req->reqId) + ": " + noMethod;
        myLog.writeInLog(noMethodLog);
        send(req->clientFd, noMethod.c_str(), noMethod.length(), 0);
      }  
     
}


//identify the cache type of the response, i is the request id, use for write log, add later
//0 for no_store, 1 for must revalidate, 2 for validation after expire
int Proxy::setCacheType(Response res){
  std::map<std::string, std::string> resBody = res.getResbody(); 
   std::string pragma = resBodyFind(resBody, "Pragma");
  std::string cacheControl = resBodyFind(resBody, "Cache-Control");
  //std::string eTag = resBodyFind(resBody, "ETag");
  if(pragma.find("no-cache") != std::string::npos){
    return 1;
  } 
  if(cacheControl.length() == 0){
    return 2;  //cache and no expire date
  }
  if(cacheControl.find("no-store") != std::string::npos || cacheControl.find("private") != std::string::npos){
    return 0;
  }
  if(cacheControl.find("age") != std::string::npos || cacheControl.find("revalidate") != std::string::npos || cacheControl.find("public") != std::string::npos){
    return 2;  //if expire, revalidate 
  }
  if(cacheControl.find("no-cache") != std::string::npos){
    return 1;
  }
  return 0;
}


//find the comment of the specific title in resBodyFind
std::string Proxy::resBodyFind(std::map<std::string, std::string> resBody, std::string item){
  std::map<std::string, std::string>::iterator it;
  it = resBody.find(item);
  if(it != resBody.end()){
    return it->second;
  }
  return "";
}


void Proxy::serverConnect(Request *req){
    std::string host = req->getHost();
    std::string port = req->getPort();
    int serverSocket = -1;
    try{
      serverSocket = serverInitSocket(host, port, req);
    }
    catch(myException &e){
      std::cout<< e.what() <<std::endl;
    }
    //**** 200 OK reponse to client
    std::string OK_200("HTTP/1.1 200 OK\r\n\r\n");

    if (send(req->clientFd, OK_200.c_str(), OK_200.length(), 0) == -1) {
        //perror("send 200 OK back failed");
        throw myException("send 200 OK back fail");
    }
    
     std::string okRes = std::to_string(req->reqId) + ": " + OK_200;
     myLog.writeInLog(okRes);
    //**** Tunnel starts ****
    int status;
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 1;
    while(true){
      std::vector<char> message(2097152);
      FD_ZERO(&readfds);
      FD_SET(req->clientFd, &readfds);
      FD_SET(serverSocket, &readfds);
      int max = req->clientFd > serverSocket? req->clientFd + 1 : serverSocket + 1;
      int rv = select(max, &readfds, NULL, NULL, &tv);
      if (rv == -1) {
        throw myException("CONNECT select() fail");
      } else if (rv == 0) {
        //printf("Timeout occurred! No data after timeout.\n");
        break;
      }
      else{
        if(FD_ISSET(req->clientFd, &readfds)){
          status = recv(req->clientFd, &message.data()[0], 2097152, 0) ;
          if(status == -1){
            //perror("receive");
            throw myException("CONNECT recv fail");
            break;
          }
          else if(status == 0){
            break;
          }   
          status = send(serverSocket, message.data(), status, 0); 
          if(status == -1){
              //perror("send");
              throw myException("CONNECT send fail");
              break;
          }           
        }
        if(FD_ISSET(serverSocket, &readfds)){
          status = recv(serverSocket, &message.data()[0], 2097152, 0);
          //std::cout<< "byteStatus:" <<status<<std::endl;
          if(status == -1){
            //perror("receive");
            throw myException("CONNECT recv fail");
            break;
          }
          else if(status == 0){
            break;
          }   
          if(status = send(req->clientFd, message.data(), status, 0) == -1){
              //perror("send");
              throw myException("CONNECT send fail");
              break;
          }           
        }       
      }
      message.clear();
    }
    close(serverSocket);
    close(req->clientFd);
    std::string tunnelClose = std::to_string(req->reqId) + ": tunnel closed\n";
    myLog.writeInLog(tunnelClose);
}

Response Proxy::communicateServer(Request *req){
  Response res;
  int serverSocket = -1;
  try{
    serverSocket = serverInitSocket(req->getHost(), req->getPort(), req);
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
  if(serverSocket == -1){
    return res;
  }
  const char *buf = req->allMessage.c_str();
  if(send(serverSocket, buf, strlen(buf) + 1, 0) == -1){
    throw myException("send request to server fail");
  }
  //std::cout<<serverSocket.sockfd<<std::endl;
  try{
    res = receiveResponse(serverSocket, req->getUrl());
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
  if (close(serverSocket) == -1) {
    throw myException("Failed to close() server socket");
  }
  return res;
}

Response Proxy::receiveResponse(int sockfd, std::string reqHeaderLine){
  std::string allResMsg;
  //std::cout<<"before recv"<<std::endl;
  while(true){
    char msg[2097152];
    memset(msg, 0, sizeof(msg));
    int byteCount = recv(sockfd, msg, 2097152, 0);
    if(byteCount == -1){
      throw myException("receive from server fail");
    }
    if(byteCount == 0){
      break;
    }
    
    allResMsg.append(msg, byteCount);
  }
    Response res(allResMsg, reqHeaderLine);
    return res;
}

void Proxy::sendResponseToClient(Response res, int clientFd){
    const char *buf = res.buf.c_str();
     int bytes_count = send(clientFd, buf, strlen(buf) + 1, 0);
}

int Proxy::serverInitSocket(const std::string &host, const std::string &port, Request *req){
  int status, serverSockfd;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;
  if(status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0){
    const char* s = "404 NOT FOUND";
    if(send(req->clientFd, s, strlen(s) + 1, 0) == -1){
      //perror("send 404 fail");
      throw myException("send 404 fail");
    }
    return -1;
  }
  struct addrinfo *p;
  for(p = res; p != NULL; p = p->ai_next){
    serverSockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (serverSockfd == -1) {
        //perror("socket");
        throw myException("serverSocketInit socket() fail");
        continue;
    }
    if (connect(serverSockfd, p->ai_addr, p->ai_addrlen) == -1) {
        //perror("connect");
        throw myException("serverSocketInit socket() fail");
        if (close(serverSockfd) == -1) {
                //perror("close() failed");
            throw myException("serverSocketInit close() fail");
        }
        continue;
    }
    break; // if we get here, we must have connected successfully
  }
  if (p == NULL) {
    // looped off the end of the list with no successful bind
    fprintf(stderr, "failed to bind socket\n");
    exit(2);
  }
  freeaddrinfo(res); // all done with this structure
  return serverSockfd;
}