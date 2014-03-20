#include "iostream"

#define TaskComplete 1
#define TaskContent 3


static bool isoutput = true;

void outputmsg(std::string msg){
    if(isoutput)
       std::cout << msg << std::endl;
}
