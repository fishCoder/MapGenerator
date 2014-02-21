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
<<<<<<< HEAD
 //   boost::thread th(mapgenerate);
 //   boost::thread th1(mapgenerate);
 //   th1.join();
  //  th.join();

    mapgenerate();
=======
    boost::thread th(mapgenerate);
    boost::thread th1(mapgenerate);
    th1.join();
    th.join();


>>>>>>> 8877b10bc23c7871b13677cb9afc00721702ccf6
    cout << "Hello world!" << endl;
    return 0;
}
