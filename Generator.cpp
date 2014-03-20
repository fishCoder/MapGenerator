#include "Generator.h"
#include "Output.h"
#include <boost/asio/yield.hpp>
#include <boost/bind.hpp>
#include <json/json.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <vector>

#include "RandomMapGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#define bufsize 1024

//char readbuf[1024];
using namespace boost::posix_time;

Generator::Generator(
        boost::asio::io_service& _io_service,
        const std::string address,
        int port,
        std::string redishost,
        int redisport,
        int dur_time_):io_service(_io_service),
                  endpoint(boost::asio::ip::address_v4::from_string(address),port)
        {
        srand((unsigned)time(NULL));
        rc.reset(new redis::client(redishost,redisport));
        this->set_dur_time(dur_time_);
        reconnect();
    }
void Generator::operator()(
    boost::system::error_code ec,
    std::size_t length){
    if(!ec){
        reenter(this){
            while(true){
                //receive task from diaspatcher
                clearbuf();
                std::cout << "async_read_some" << std::endl;
                yield psocket->async_read_some(boost::asio::buffer(readbuf,bufsize),*this);
                //start task
                HandleTask(readbuf);
                //notify diaspatcher that i done
                std::cout << "task_ok:" << task_ok << std::endl;
                yield psocket->async_write_some( boost::asio::buffer(task_ok.c_str(),task_ok.length()),*this);

            }
        }
    }else{
        psocket->close();
        reconnect();
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
    int data_size = 0;
    for(int i=0; i<num ;  i++){
        std::string map_data;
        std::string npc_data;
        ptime gen_start = microsec_clock::local_time();
        HeroX::RandomMapGenerator::Generate(rand(), type, map_data, npc_data);
        ptime gen_end   = microsec_clock::local_time();
        time_duration gen_duration = gen_end - gen_start;
        int gen_duration_ = gen_duration.total_microseconds()/1000;
        is_over_time(gen_duration_,type);
        std::cout << "[gen time :" << gen_duration_ << " ms" <<"]"<< std::endl;

        sendMap(map_data,npc_data,type);
        data_size += (map_data.length()+npc_data.length());


    }
    Json::Value root;
    Json::FastWriter writer;

    root["type"] = 1;
    root["id"]   = id;
    root["map"]  = type;
    root["amount"] = num;
    root["size"] = data_size;

    task_ok=writer.write(root);
}
void Generator::is_over_time(int gen_duration,int map_type_id){
    if(gen_duration<this->dur_time)return;
    std::cout << "over time " << this->dur_time <<std::endl;
    Json::Value root;
    Json::FastWriter writer;

    root["type"] = 2;
    root["id"]   = id;
    root["map"]  = map_type_id;
    root["duration"] = gen_duration;

    std::string task_ok=writer.write(root);
    std::cout << "task_ok:" << task_ok << std::endl;
    psocket->async_write_some( boost::asio::buffer(task_ok.c_str(),task_ok.length()),boost::bind(&Generator::write_handler,this,_1,_2));

}
void Generator::sendMap(std::string & rmap,std::string & npc_data, int type){
    std::string zero = "0";
    std::string use  = "use";
    std::string map_data = "map";
    std::string npc = "npc";
    std::string code = "code";
    typedef std::pair<std::string,std::string> hash_pair;
    std::vector<hash_pair> vect;
    boost::posix_time::ptime now=boost::posix_time::microsec_clock::universal_time();
    int key = abs((now.time_of_day().total_microseconds()<<32)>>32);
    //std::cout << key << std::endl;
    //std::cout << (itr->data).length() << std::endl;
    std::string _key = boost::lexical_cast<std::string>(key);
    std::string _type = boost::lexical_cast<std::string>(type);
    rc->sadd(_type,_key);
    vect.push_back(hash_pair(use,zero));
    vect.push_back(hash_pair(code,boost::lexical_cast<std::string>(rand())));
    vect.push_back(hash_pair(map_data,rmap));
    vect.push_back(hash_pair(npc,npc_data));
    rc->hmset(_key,vect);
    //rc->hset(_key);
}

void Generator::write_handler(boost::system::error_code ec,std::size_t){
    if(ec){
        psocket->close();
        reconnect();
    }
}
void Generator::set_dur_time(int durtime){
    this->dur_time = durtime;
}
void Generator::reconnect(){
    psocket.reset(new tcp::socket(io_service));
    boost::system::error_code ec;
    do{
        std::cout << "Try to connect Dispatcher" << std::endl;
        psocket->connect(endpoint,ec);
        sleep(1);
    }while(ec);
    outputmsg("连接调度器成功");
    psocket->read_some(boost::asio::buffer(&id,sizeof(int)));
    std::cout << "get id : " << id << std::endl;

    Json::Value root;
    Json::FastWriter writer;

    root["type"] = 1;
    root["id"]   = id;
    task_ok = writer.write(root);

    readbuf = new char[1024];

    (*this)();
}
