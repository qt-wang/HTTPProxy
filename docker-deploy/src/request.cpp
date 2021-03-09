#include "request.h"

Request::Request(int clientSockfd, std::string ip, std::string time): clientFd(clientSockfd), clientIp(ip), requestTime(time){  
}

Request::Request(int clientSockfd, std::string ip, std::string time, int id_for_req): clientFd(clientSockfd), clientIp(ip), requestTime(time), reqId(id_for_req){  
}

Request:: ~Request(){}

void Request::setUp(std::string message, std::string time){
  setHeaderLine(message);
  setUrl(message);
  setMethod(message);
  setAllMessage(message);
  setBody(message);
  setHost();
  setPort();
  setRequestTime(time);
}

//set the first line of a request
void Request::setHeaderLine(std::string message){
  size_t headerEnd = message.find("\r\n");
  //std::cout<< "message:" << message<<std::endl;
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type");
  }
  headerLine = message.substr(0, headerEnd);
}

std::string Request::getHeaderLine(){
  return headerLine;
}

void Request::setUrl(std::string message){
  size_t headerEnd = message.find("\r\n");
  //std::cout<< "message:" << message<<std::endl;
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type");
  }
  std::string headerLine = message.substr(0, headerEnd);
  size_t temp1 = headerLine.find(" ") + 1;
  std::string noMethodHeader = headerLine.substr(temp1);
  reqUrl = noMethodHeader.substr(0, noMethodHeader.find(" "));
}

std::string Request::getUrl(){
  return reqUrl;
}

void Request::setMethod(std::string headerLine){
  size_t methodEnd = headerLine.find(" ");
  if(methodEnd == std::string::npos){
    throw myException("wrong request header type");
  }
  method = headerLine.substr(0, methodEnd);
}

std::string Request::getMethod(){
  return method;
}

void Request::setBody(std::string message){
  size_t headerEnd = message.find("\r\n");
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type");
  }
  size_t secondLineHeader = headerEnd + 2;
  size_t messageEnd = message.find("\r\n\r\n");
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type with no end");
  }
  std::string body = message.substr(headerEnd + 2, messageEnd);
  //size_t temp = 0;
  size_t bodyEnd = body.find("\r\n\r\n");
  if(bodyEnd == std::string::npos){
    throw myException("wrong request message body type with no end");
  }
  size_t bodyPtr = 0;
  while(bodyPtr < bodyEnd){
    //std::cout << "bodyPtr number: "<< bodyPtr << std::endl;
    size_t bodySingleLineEnd = body.find("\r\n", bodyPtr);
    //std::cout << "bodySingleLineEnd number: "<<bodySingleLineEnd << std::endl;
    if(bodySingleLineEnd == std::string::npos){
      throw myException("wrong request message body single line type with no end");
    }
    std::string singleLine = body.substr(bodyPtr, bodySingleLineEnd - bodyPtr);
    //std::cout << body.at(bodyPtr) << "----"<< body.at(bodySingleLineEnd - 1) << std::endl;
    //std::cout << "single line thing: "<<singleLine << "&&&&&&" <<singleLine.length() <<"@@@@@" << std::endl;
    size_t bodySingleLineMiddle = singleLine.find(":");
    if(bodySingleLineMiddle == std::string::npos){
      throw myException("wrong request message body single line type with no :");
    }
    //title
    std::string title = singleLine.substr(0, bodySingleLineMiddle);
    bodySingleLineMiddle += 2;
    std::string comment = singleLine.substr(bodySingleLineMiddle);
   // std::cout << "map thing "<<title  << " + " << comment << std::endl;
    reqBody[title] = comment;
    //body.insert(std::pair<std::string, std::string>(title, comment)); 
     bodyPtr += singleLine.length() + 2; 
  }
  //std::cout << "body size: " << reqBody.size() << std::endl;
}

void Request::setAllMessage(std::string message){
  allMessage = message;
}

std::string Request::getAllMessage(){
  return allMessage;
}

std::string Request::getHostComment(){
  std::map<std::string, std::string>::iterator it;
  it = reqBody.find("Host");
  if(it == reqBody.end()){
    throw myException("no host in body map\n");
  }
  //std::cout << "host comment:" << it-> second << std::endl;
  return it->second;
}

void Request::setHost(){
  std::string hostComment = getHostComment();
  size_t middle = hostComment.find(":");
  if(middle == std::string::npos){
      host = hostComment;
  }
  else{
    host = hostComment.substr(0, middle);
  }
}

std::string Request::getHost(){
  return host;
}
void Request::setPort(){
  std::string hostComment = getHostComment();
  size_t middle = hostComment.find(":");
  if(middle == std::string::npos){
      port = "80";
  }
  else{
    port = hostComment.substr(middle + 1);
  }
}

std::string Request::getPort(){
  return port;
}


void Request::setRequestTime(std::string time){
  requestTime = time;
}

std::string Request::getRequestTime(){
  return requestTime;
}
  
  
void Request::addToBody(std::string title, std::string comment){
  reqBody[title] = comment;
  std::string updatedAllMessage = getAllMessage();
  size_t end = updatedAllMessage.find("\r\n\r\n");
  end += 2;
  std::string addLine = title + ": " + comment + "\r\n";
  updatedAllMessage.insert(end, addLine);
  setAllMessage(updatedAllMessage);
}
/*
  //std::string bodySet[]= body.split("\r\n", temp_end);
  for(std::string str : bodySet){
    size_t titleEnd = str.find(":");
    std::string title = str.substr(0, titleEnd);
    titleEnd += 2;
    std::string comment = str.substr(titleEnd);
    body[title] = comment;
  }
  std::cout << "body size : "<<body.size() << std::endl; */
