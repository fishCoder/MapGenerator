#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "RandomMapGenerator.h"
#include "Generator.h"
#include "INIReader.h"
using namespace std;

int main(int agrc ,char * argv[])
{

    INI ini("gen.ini");
    string file_path = ini.get("gen.csv");
    string redishost = ini.get("gen.redisip");
    int redisport = ini.get_int("gen.redisport");
    int durtime = ini.get_int("gen.durtime");

    string dip = ini.get("gen.dip");
    int dport = ini.get_int("gen.dport");
    if(HeroX::RandomMapGenerator::LoadDataFromCSV(file_path.c_str()) != HeroX::RandomMapGenerator::GeneratorResult::SUCCESS)
		return false;

    boost::asio::io_service io_ser;
    Generator * en= new Generator(io_ser,dip,dport,redishost,redisport,durtime);
    io_ser.run();

 /*   boost::thread_group threadGroup;
    int i=0;
    for(;i<2;i++){
        threadGroup.create_thread(mapgenerate);
    }
    threadGroup.join_all();*/

    return 0;
}
