#ifndef PROXY_MYEXCEPTION_H
#define PROXY_MYEXCEPTION_H
#include "headers.h"
/*class ErrorException : public std::exception {
    private:

        std::string message;
    public:
        explicit ErrorException(std::string message): message(message){}

        virtual const char *what(){
            return message.c_str();
        }
    };*/
    
  
class myException :public std::exception
{
public:
	myException(std::string c)
	{
		m_p = c;
	}
	//what()函数返回错误信息
	virtual const char* what()
	{
		return m_p.c_str();
	}
private:
	std::string m_p;
};
#endif 