#include "Generator.h"
#include "Output.h"
#include <boost/asio/yield.hpp>
#include "json/json.h"
#include "stdio.h"
#include <boost/lexical_cast.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#define bufsize 1024

//char readbuf[1024];


Generator::Generator(
        boost::asio::io_service& io_service,
        const std::string address,
        int port)
        {

        psocket.reset(new tcp::socket(io_service));
        tcp::endpoint endpoint(boost::asio::ip::address_v4::from_string(address),port);
        boost::system::error_code ec;
        psocket->connect(endpoint,ec);
        if(!ec){
            outputmsg("连接调度器成功");
            psocket->read_some(boost::asio::buffer(&id,sizeof(long)));
            std::cout << "get id : " << id << std::endl;
            rc.reset(new redis::client(redishost,redisport));
            std::cout << "get id : " << id << std::endl;

            Json::Value root;
            Json::FastWriter writer;

            root["type"] = 1;
            root["id"]   = id;
            task_ok = writer.write(root);

            readbuf = new char[1024];

            (*this)();
        }
        else{
            outputmsg("连接调度器失败");
        }

    }
void Generator::operator()(
    boost::system::error_code ec,
    std::size_t length){
    if(!ec){
        reenter(this){
            while(true){
                //receive task from diaspatcher
                std::cout << "async_read_some" << std::endl;
                yield psocket->async_read_some(boost::asio::buffer(readbuf,bufsize),*this);
                //start task
                HandleTask(readbuf);
                //notify diaspatcher that i done
                std::cout << "async_write_some" << std::endl;
                yield psocket->async_write_some( boost::asio::buffer(task_ok.c_str(),task_ok.length()),*this);
            }
        }
    }
}
void Generator::HandleTask(std::string task){
    Json::Reader reader;
    Json::Value root;
    std::cout << "Generator::HandleTask() : " << task <<std::endl;
    if(reader.parse(task,root)){
        switch(root.get("type",-1).asInt()){
        case TaskContent:
            std::cout << "TaskContent" << std::endl;
            creatMap(root.get("map",-1).asInt(),root.get("num",-1).asInt());
        break;
        }
    }
}
void Generator::clearbuf(){
    memset(readbuf,'\0',1024);
}

void Generator::creatMap(int type,int num){
    std::vector<Randmap> vecMap;
    for(int i=0; i<num ;  i++){
        Randmap rmap;
        rmap.id = type;
        rmap.data = fieldmapGenerator.generateMap(2,type);
        vecMap.push_back(rmap);
    }
    sendMap(vecMap);
}
void Generator::sendMap(std::vector<Randmap> &maps){
    std::vector<Randmap>::iterator itr;
    for(itr = maps.begin() ; itr!=maps.end() ; itr++){
        boost::posix_time::ptime now=boost::posix_time::microsec_clock::universal_time();
        long key = now.time_of_day().total_microseconds();
        std::cout << key << std::endl;
        std::cout << (itr->data).length() << std::endl;
        rc->set(boost::lexical_cast<std::string>(key),itr->data);
        rc->sadd(boost::lexical_cast<std::string>(itr->id),boost::lexical_cast<std::string>(key));

    }
    maps.clear();
}
