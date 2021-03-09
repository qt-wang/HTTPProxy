#include "cache.h"
/*
  int size;
  std::map<std::string, Response> responseMap;
  std::list<Response> cacheList;
*/
Cache::Cache(){
  size = 0;
}

Cache::~Cache(){}

//with a given request head, find if the corresponding response is in cache
Response* Cache::find(std::string requestUrl){
  std::map<std::string, Response*>::iterator it;
  it = responseMap.find(requestUrl);
  //no corresponding cache bofore
  if(it != responseMap.end()){
  //Response *res;
    //res = it -> second;
    //cacheList.remove(requestHead);
    //cacheList.push_front(requestHead);
    return it->second;
    
  }
  else{
    return NULL;
  }
}
  
//remove the least used cache(end of the list)
void Cache::remove(){
  std::string endReqStr = cacheList.back();
  cacheList.pop_back();
  responseMap.erase(endReqStr);
  size --;
  //std::cout << "go!" << std::endl;
}
  
//add the new cacheElement to the cache
void Cache::add(Response *res){
  std::string matchedReqUrl = res->getMatchedReqUrl();
  if (size < 2){ //the maxumun size of the cache can be modified here 
    responseMap[matchedReqUrl] = res;
    cacheList.push_front(matchedReqUrl);
    size ++;
  }
  else{
    remove();
    //question:当两个request的headerline一模一样时，该怎么办？
    cacheList.push_front(matchedReqUrl);
    responseMap[matchedReqUrl] = res;
  }
}

void Cache::updateToMostUse(Response *res){
   std::string requestUrl = res->getMatchedReqUrl();
   cacheList.remove(requestUrl);
   cacheList.push_front(requestUrl);
}