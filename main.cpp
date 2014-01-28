#include <iostream>
#include "boost/asio/io_service.hpp"
#include "boost/thread/thread.hpp"
#include "boost/bind.hpp"
#include "Generator.h"
using namespace std;

void mapgenerate(){
    boost::asio::io_service io_ser;
    Generator * en= new Generator(io_ser,"127.0.0.1",9010);
    io_ser.run();
}

int main()
{
    boost::thread th(mapgenerate);
    boost::thread th1(mapgenerate);
    th1.join();
    th.join();


    cout << "Hello world!" << endl;
    return 0;
}
