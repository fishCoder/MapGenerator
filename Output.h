#include "iostream"

#define TaskComplete 1
#define TaskContent 3

#define redishost "127.0.0.1"
#define redisport 6379

static bool isoutput = true;

void outputmsg(std::string msg){
    if(isoutput)
       std::cout << msg << std::endl;
}
