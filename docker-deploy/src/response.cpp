#include "response.h"
Response::Response(){}

Response::Response(std::string resAll, std::string reqUrl):buf(resAll), matchedReqUrl(reqUrl){
  setHeaderLine(); 
  setStatusCode();
  setResBody();
  birthTime = std::time(nullptr);  //set birth time
  //last_Modified = "";
  //etag == "";
  lastModified = bodyFind("Last-Modified");
  etag = bodyFind("ETag");
}

Response::~Response(){}


void Response::setHeaderLine(){
  std::string headerLine;
  size_t headerEnd = buf.find("\r\n");
  if(headerEnd == std::string::npos){
    throw myException("wrong response message type");
  }
  headerLine = buf.substr(0, headerEnd);
  resHeaderLine = headerLine;
}

std::string Response::getHeaderLine(){
  return resHeaderLine;
}


void Response::setStatusCode(){
  std::string headerLine = getHeaderLine();
  if(headerLine.length() == 0){
    throw myException("no header line in response");
  }
  statusCode = headerLine.substr(headerLine.find(" ") + 1, 3);
  //return statusCode;
}

std::string Response::getStatusCode(){
  return statusCode;
}

/*
void Response::setBuf(std::string message){
  buf = message;
}

std::string Response::getBuf(){
  return buf;
}*/

void Response::setResBody(){
  size_t headerEnd = buf.find("\r\n");
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type");
  }
  size_t secondLineHeader = headerEnd + 2;
  size_t messageEnd = buf.find("\r\n\r\n");
  if(headerEnd == std::string::npos){
    throw myException("wrong request message type with no end");
  }
  std::string body = buf.substr(headerEnd + 2, messageEnd);
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
    //std::cout << "res map thing "<<title  << " + " << comment << std::endl;
    resBody[title] = comment;
    //body.insert(std::pair<std::string, std::string>(title, comment)); 
     bodyPtr += singleLine.length() + 2; 
  }
  //std::cout << "body size: " << reqBody.size() << std::endl;
}

std::map<std::string, std::string> Response::getResbody(){
  return resBody;
}

std::string Response::getBuf(){
  return buf;
}
/*
void Response::setMatchedReqHead(std::string reqHead){
  matchedReqHead = reqHead;
}*/

std::string Response::getMatchedReqUrl(){
  return matchedReqUrl;
}

void Response::setCacheValue(int cacheTypeNum){  //set age and time to the response if it should be cached; =200 OK
  cacheType = cacheTypeNum;
  if(cacheType == 1){
    age = 0;
  }
  else if(cacheType == 2){
    std::string cacheControl = bodyFind("Cache-Control");
    size_t SMaxAgeIndex = cacheControl.find("s-maxage");
    size_t maxAgeIndex = cacheControl.find("max-age");
    std::string expire = bodyFind("Expires");
    //std::string last_
    if(cacheControl == ""){
      expireTime = INT_MAX;  //no expire date
    }
    else if(SMaxAgeIndex != std::string::npos){    //s-maxage exist, set s-maxage 
    std::cout << "in s-maxage" <<std::endl;
      SMaxAgeIndex += 9;
      std::string SAgeStr = cacheControl.substr(SMaxAgeIndex);  //get age string
      age = ageStrtoAge(SAgeStr);  //convert age string to int
      expireTime = birthTime + age;
      std::cout << "in age age: " << age << std::endl;
      std::cout << "in age birthtime: " << birthTime << std::endl;
      std::cout << "in age expire time: " <<expireTime <<std::endl;
    }
    else if(maxAgeIndex != std::string::npos){  //S-age not exist, but max-age exist
      std::cout << "in max-age" <<std::endl;
      maxAgeIndex += 8;
      std::string maxAgeStr = cacheControl.substr(maxAgeIndex);  //get age string
      age = ageStrtoAge(maxAgeStr);  //convert age string to int
      expireTime = birthTime + age;
      std::cout << "in age age: " << age << std::endl;
      std::cout << "in age birthtime: " << birthTime << std::endl;
      std::cout << "in age expire time: " <<expireTime <<std::endl;
    }
    else if(expire != ""){//no age, find expire
    std::cout << "in Expire" <<std::endl;
      expireTime = timeStr_to_timeT(expire);  //set expire time
      std::cout << "current time: " <<  std::time(nullptr) << std::endl;
      std::cout << "expire time: " <<  expireTime << std::endl;
      age = expireTime - birthTime;      
    }
    else{  //no age, no expire, set it to type 1
      cacheType = 1;
      age = 0;
    }
  }  
}

//find the comment of the specific title in resBodyFind
std::string Response::bodyFind(std::string item){
  std::map<std::string, std::string>::iterator it;
  it = resBody.find(item);
  if(it != resBody.end()){
    return it->second;
  }
  return "";
}

int Response::ageStrtoAge(std::string ageStr){
  int n = 0;
  for(int i = 0; i < ageStr.length(); i ++){
    if(ageStr[i] != '\r' && ageStr[i] != '\n'){
      int temp = ageStr[i] - '0';
      n = n * 10 + temp;
    }
  }
  return n;
}

std::time_t Response::timeStr_to_timeT(std::string time) {
  struct tm t;
  if(strptime(time.c_str(), "%a, %d %b %Y %H:%M:%S", &t) == NULL){
    std::cerr << "strptime failed" << std::endl;
  }
  else {
    t.tm_isdst = -1;
    std::time_t time = mktime(&t);
    if (time != -1) {
      return time;
    }
  }
  return -1;
} 

std::string Response::getLastModified(){
  return lastModified;
}

std::string Response::getEtag(){
    return etag;
}






