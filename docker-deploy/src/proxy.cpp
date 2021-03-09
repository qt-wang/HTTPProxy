#include "proxy.h"
#include "cache.h"
#include "myexception.h"
Proxy::Proxy(int id, int portNum):requestId(id), port(portNum){}

Proxy::~Proxy(){}


void Proxy::beginService(){
  //while(true){
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
//      return EXIT_FAILURE;
    }
    
    try{
      this->sock.myListen();
      std::cout << "ready to listen" << std::endl;
    }
    catch(myException &e){
      std::cout<< e.what() << std::endl;
//      return EXIT_FAILURE;
    }   
    while(true){
      Request *req = this->getRequest(requestId);
      std::cout<< "req clientFd:" <<req->clientFd<<std::endl;
      std::cout<< "req clientIp:" <<req->clientIp<<std::endl;
      std::cout<< "req time:" << req->time<<std::endl;
      this->parseRequest(req);
      //char client_ip_addr[INET_ADDRSTRLEN];
        requestId ++;
      
      std::thread t(&Proxy::processRequest, this , req);
      t.detach();
      //this->processRequest(req);
    }
}

Request *Proxy::getRequest(int requestIdNum){
  int clientFd;
  struct sockaddr_in their_addr;  //本来用的是书上的storage，但是为了得到.sin_addr, 只能改成_in
  memset(&their_addr, 0, sizeof (their_addr));
  socklen_t addr_size = sizeof(their_addr);
//  while(true){
    try{
//      std::cout << "before accept" << std::endl;
//      std::cout << sock.sockfd << std::endl;
       clientFd = sock.myAccept(their_addr, addr_size);     
//      std::cout << "after accept" << std::endl;
//      break;
/**************************to be modified(used for log)*******************************/
      std::time_t seconds = std::time(NULL);
        std::string request_time = std::string(std::asctime(std::gmtime(&seconds)));
        request_time = request_time.substr(0, request_time.find("\n"));

        char client_ip_addr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(their_addr.sin_addr), client_ip_addr, INET_ADDRSTRLEN) == NULL) {
            throw myException("inet_ntop() failed");
        }
        std::string client_ip = std::string(client_ip_addr);
/*************************************************************************/
        //Request client_request;
        Request *client_request = new Request(clientFd, client_ip, request_time, requestIdNum);
        //client_request.id = ;
        //client_request.clientFd = clientFd;
        //client_request.clientIp = client_ip;
        //client_request.time = request_time;
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
  std::cout << byte_count << std::endl << buf << std::endl;
  if(byte_count == 0){
    return;
  }
  std::string request_message;
  std::stringstream stream;
  stream << buf;
  request_message = stream.str();
  //std::cout<< "request message:"<< request_message<<std::endl;
//  req.setAllMessage(request_message);
//  req.setHeaderLine(request_message);
//  req.setMethod(req.headerLine);

  //std::cout << "header line is: " << req.getHeaderLine() << std::endl;
  //std::cout << "Method is: " << req.getMethod() << std::endl;
  //std::cout << "All message is: " << req.getAllMessage() << std::endl;
//  req.setBody(request_message);
  //std::cout<< "request message before setup:"<< request_message<<std::endl;
  try{
    std::time_t requestTime = std::time(nullptr);
    std::string requestTimeSrtPre = std::string(std::asctime(std::gmtime(&requestTime)));
    std::string requestTimeSrt = requestTimeSrtPre.substr(0, requestTimeSrtPre.find("\n"));
    req->setUp(request_message, requestTimeSrt);
    //std::cout << "request id = " << requestId << std::endl;
    std::string reqSetUpLog = std::to_string(req->reqId) + ": \"" + req->getHeaderLine() + "\" from " + req->clientIp + " @ " + req->getRequestTime() + "\n";
    myLog.writeInLog(reqSetUpLog);
    //std::cout << "log test: " <<reqSetUpLog << std::endl;
  }
  catch(myException &e){
    std::cout<< e.what() << std::endl;
  }
  //std::cout<< "request message after setup:" << req.getAllMessage()<<std::endl;
  //identify method
  
  //req.getmethod()
}

void Proxy::processRequest(Request *req){
    //int i = 0;  
    std::cout << "all request: " << req->allMessage << std::endl;
    std::cout<<"host :"<< req->getHost()<<std::endl;
    std::cout<<"port :"<< req->getPort()<<std::endl;
    std::cout<<"Url :"<< req->getUrl()<<std::endl;
    // POST method
    std::cout<<"method:"<< req->getMethod()<<std::endl;
    //if(req.getMethod() == "POST" && i == 1){
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
        //log报错
          std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(warning);
          sendResponseToClient(res, req->clientFd);
      }
      else if(postStatusCode[0] == '4'){
        //log 报错
        std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }
      else if(postStatusCode[0] == '5'){
      //log报错
        std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }
      else{
        //log 未知错误
        std::string error = std::to_string(req->reqId) + ": ERROR Unknown error\n";
        myLog.writeInLog(error);
        sendResponseToClient(res, req->clientFd);
      }    
      if (close(req->clientFd) == -1) {
        perror("client socket close() failed");
      }
    std::cout<<"response statusCode"<<std::endl<<res.statusCode<<std::endl;
    std::cout<<"response matchedReqUrl"<<std::endl<<res.matchedReqUrl<<std::endl;
    std::cout<<"response resHeaderLine"<<std::endl<<res.resHeaderLine<<std::endl;
    std::cout<<"response buf"<<std::endl<<res.buf<<std::endl;
    
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
    //std::cout << "---------------------------------------------------------------------------------------------------------------------" << std::endl;
      std::string testResStr = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: public, max-age=86400\r\nExpires: Fri, 11 Jun 2021 11:33:01 GMT\r\nX-Cloud-Trace-Context: a5d237eac4db95e0d3850364546e1d2b\r\nDate: Wed, 03 Mar 2021 09:15:51 GMT\r\nServer: Google Frontend\r\nContent-Length: 17\r\n\r\nhtml code\r\nhtml1 code 2\r\nhtml code 1\r\n";
    /*
       //_______________________________________________________________________________________________________________________________________test Cache 
       "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nCache-Control: public, max-age=86400\r\nPragma: no-cache\r\nAccess-Control-Allow-Origin: *\r\nX-Cloud-Trace-Context: a5d237eac4db95e0d3850364546e1d2b\r\nDate: Wed, 03 Mar 2021 09:15:51 GMT\r\nServer: Google Frontend\r\nContent-Length: 17\r\n\r\nhtml code\r\nhtml1 code 2\r\nhtml code 1\r\n";
       
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

      std::cout << << std::endl;
      singleCache.add(&res); 
      */
//_______________________________________________________________________________________________________________________________________test Cache 
      //variable: singleCache, response
      //Cache singleCache;
      std::string reqUrl = req->getUrl();
      Response * resInCache = singleCache.find(reqUrl);
      if(resInCache == NULL){  //cache miss
        std::cout <<"cache NULL" << std::endl;
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
        std::cout << res.getBuf() << std::endl;
        std::string resAllMessage = res.getBuf();
        //when response is enpty
        if(resAllMessage.length() == 0){
      //perror no message in resAllmessage
        //std::cout << "res is 0!!!!" << std::endl;
          //return;
          throw myException("The response's length is 0!");
        }
        std::string statusCode = res.getStatusCode();
        if(statusCode== "200"){  //response success
        //待会写判断是否缓存
          int cacheType = setCacheType(res);
          if(cacheType == 1 || cacheType == 2){
            std::cout << "cacheType" << cacheType << std::endl;
            res.setCacheValue(cacheType);
            singleCache.add(&res);
            //std::string cacheLog = std::to_string(req->reqId) + ": Received \"" + res.getHeaderLine() + "\" from " + req.reqUrl + "\n";
            //myLog.writeInLog(cacheLog);
            //log
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
            //log
            //std::cout << "add to cache" << std::endl;
            //std::cout << "res age: " << res.age << "+" << res.expireTime << std::endl;
          }
          else{  //cacheType = 0, no store
            std::string NoStoreLog = std::to_string(req->reqId) + ": not cacheable because it is a No_Store type\n";
            myLog.writeInLog(NoStoreLog);
          }
        //senf back to client
        //std::cout<< "response in GET:" << res.getBuf()<<std::endl;
        std::string sendToClientInGETMiss = std::to_string(req->reqId) + ": Responding \"" + res.getHeaderLine()  + "\"\n";
        myLog.writeInLog(sendToClientInGETMiss);
        sendResponseToClient(res, req->clientFd);
        //std::cout << "send back" << std::endl;
        }
        else if(statusCode == "304"){
          //std::cout << "304!!!" << std::endl;
      //log: print 服务器已经缓存了请求，此时不用cache，但也不是error
      //如何显示为改变的前一个request？待解决
      sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '3'){
        //log报错
          std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(warning);
          sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '4'){
        //log 报错
          std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }
        else if(statusCode[0] == '5'){
        //log报错
          std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }
        else{
      //log 未知错误
          std::string error = std::to_string(req->reqId) + ": ERROR Unknown error\n";
          myLog.writeInLog(error);
          sendResponseToClient(res, req->clientFd);
        }    
    //在miss时，无论什么情况都要将服务器发来的response发送给client
    //待写：~~~~~~
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
          //发送给server, get new response
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
          //待会写判断是否缓存
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
      //log: print 服务器已经缓存了请求，此时不用cache，但也不是error
      //如何显示为改变的前一个request？待解决
            singleCache.updateToMostUse(resInCache);
          //send back to client
            //std::cout<< "response in GET:" << resInCache->getBuf()<<std::endl;
            std::string getNoneChangeBack = std::to_string(req->reqId) + ": Responding \"" + resInCache->getHeaderLine()  + "\"\n";
            myLog.writeInLog(getNoneChangeBack);
            sendResponseToClient(*resInCache, req->clientFd);
          
          }
          else if(statusCode[0] == '3'){
          //log报错
            std::string warning = std::to_string(req->reqId) + ": WARNING " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(warning);
            sendResponseToClient(res, req->clientFd);
          }
          else if(statusCode[0] == '4'){
          //log 报错
            std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(error);
            sendResponseToClient(res, req->clientFd);
          }
          else if(statusCode[0] == '5'){
          //log报错
            std::string error = std::to_string(req->reqId) + ": ERROR " + res.getHeaderLine()  + "\n";
            myLog.writeInLog(error);
            sendResponseToClient(res, req->clientFd);
          }
          else{
      //log 未知错误
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
//std::string cacheType(Response res, int i){
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
    //CONNECT 默认端口443， 要改一下request类里面的默认设置，改成如果没有port则port属性变为空字符串
    //if(port == ""){
    //  port = "443";
    //}
    int serverSocket = -1;
    try{
      serverSocket = serverInitSocket(host, port, req);
    }
    catch(myException &e){
      std::cout<< e.what() <<std::endl;
    }
    //**** 200 OK reponse to client
    std::string OK_200("HTTP/1.1 200 Connection Established\r\n\r\n");

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
        //perror("select"); // error occurred in select()
        throw myException("CONNECT select() fail");
      } else if (rv == 0) {
        //printf("Timeout occurred! No data after timeout.\n");
        break;
      }
      else{
        if(FD_ISSET(req->clientFd, &readfds)){
          status = recv(req->clientFd, &message.data()[0], 2097152, 0) ;
          //std::cout<< "byteStatus:" <<status<<std::endl;
          if(status == -1){
            //perror("receive");
            throw myException("CONNECT recv fail");
            break;
          }
          else if(status == 0){
            break;
          }   
          status = send(serverSocket, message.data(), status, 0); 
          //std::cout<< "byteStatus:" <<status<<std::endl;
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
    //std::cout << "before msg rece" << std::endl;
    int byteCount = recv(sockfd, msg, 2097152, 0);
    //std::cout << "after msg rece" << std::endl;
    //std::cout << byteCount << std::endl;
    //std::cout<<errno<<std::endl;
    if(byteCount == -1){
      throw myException("receive from server fail");
    }
    if(byteCount == 0){
      break;
    }
    
    allResMsg.append(msg, byteCount);
//    std::cout<< "res.buf"<< res.buf <<std::endl;
  }
/*  std::cout<<"receiveResponse sockfd:"<< sockfd<<std::endl;
  char msg[4096];
  int byteCount = recv(sockfd, msg, 4096, 0);
  std::cout<<byteCount<<std::endl;
  res.buf = msg;
  std::cout<<"after recv"<<std::endl;*/
    Response res(allResMsg, reqHeaderLine);
    return res;
}

void Proxy::sendResponseToClient(Response res, int clientFd){
    //std::cout<<"before send"<<std::endl;
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
    //std::cout<<"socket.cpp sockfd:"<<serverSockfd<<std::endl;
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