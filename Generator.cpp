#include "Generator.h"
#include "Output.h"
#include <boost/asio/yield.hpp>
#include <boost/bind.hpp>
#include <json/json.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <unistd.h>

#include "RandomMapGenerator.h"
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
                //读头

                psocket->receive(boost::asio::buffer(&read_len,sizeof(unsigned int)));
                std::cout << "async_read_head : " << read_len << std::endl;
                //读包体
                psocket->receive(boost::asio::buffer(readbuf,read_len));
                std::cout << "async_read_body : " << readbuf << std::endl;

                //start task
                HandleTask(readbuf);
                //notify diaspatcher that i done
                yield psocket->async_send(boost::asio::buffer(sendbuf,send_len),*this);

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

void Generator::creatMap(int map_type,int num){
    int data_size = 0;
    for(int i=0; i<num ;  i++){
        std::string map_data;
        std::string npc_data;
        ptime gen_start = microsec_clock::local_time();
        HeroX::RandomMapGenerator::Generate(rand(), map_type, map_data, npc_data);
        ptime gen_end   = microsec_clock::local_time();
        time_duration gen_duration = gen_end - gen_start;
        int gen_duration_ = gen_duration.total_microseconds()/1000;
        is_over_time(gen_duration_,map_type);
        std::cout << "[gen time :" << gen_duration_ << " ms" <<"]"<< std::endl;

        sendMap(map_data,npc_data,map_type);
        data_size += (map_data.length()+npc_data.length());


    }
    Json::Value root;
    Json::FastWriter writer;
    /**
    *type 调度器根据type以确定该消息包如何处理 1表示生成器完成一个任务
    *id 表示该生存器的在服务器的id
    *map 表示地图的类型
    */
    root["type"] = 1;
    root["id"]   = id;
    root["map"]  = map_type;
    root["amount"] = num;
    root["size"] = data_size;

    std::string task_ok=writer.write(root);
    unsigned int len = task_ok.length();
    memcpy(sendbuf,&len,sizeof(unsigned int));
    memcpy(sendbuf+sizeof(unsigned int),task_ok.c_str(),len);
    send_len =  len+sizeof(unsigned int);
}
void Generator::is_over_time(int gen_duration,int map_type_id){
    if(gen_duration<this->dur_time)return;
    std::cout << "over time " << this->dur_time <<std::endl;
    Json::Value root;
    Json::FastWriter writer;

    /**
    *type = 2 表示生成一张地图所用时间超过设定的时间，将这个消息反馈给调度器以记录
    *duration 生成一张地图的耗时
    */

    root["type"] = 2;
    root["id"]   = id;
    root["map"]  = map_type_id;
    root["duration"] = gen_duration;

    std::string task_ok=writer.write(root);
    unsigned int len = task_ok.length();
    memcpy(sendbuf,&len,sizeof(unsigned int));
    memcpy(sendbuf+sizeof(unsigned int),task_ok.c_str(),len);
    send_len = len+sizeof(unsigned int);
    psocket->async_send( boost::asio::buffer(sendbuf,send_len),boost::bind(&Generator::write_handler,this,_1,_2));

}
void Generator::sendMap(std::string & rmap,std::string & npc_data, int map_type_id){
    std::string zero = "0";
    std::string use  = "use";
    std::string map_data = "map";
    std::string npc = "npc";
    std::string code = "code";
    typedef std::pair<std::string,std::string> hash_pair;
    std::vector<hash_pair> vect;
    //std::cout << key << std::endl;
    //std::cout << (itr->data).length() << std::endl;

    /**
    *地图实例id(key)：ins_xxxxxxxxxx
    */
    std::stringstream ss_map_ins_key;
    ss_map_ins_key << "ins_" << ++map_instance_id;

    /**
    *地图类型id(key)：map_type_xxxx
    */
    std::stringstream ss_map_type;
    ss_map_type << "map_type_" << map_type_id;
    /**
    *向set中存放地图实例id
    */
    rc->sadd(ss_map_type.str(),ss_map_ins_key.str());
    /**
    *使用次数
    */
    vect.push_back(hash_pair(use,zero));
    /**
    *验证码
    */
    vect.push_back(hash_pair(code,boost::lexical_cast<std::string>(rand())));
    /**
    *地图数据
    */
    vect.push_back(hash_pair(map_data,rmap));
    /**
    *npc数据
    */
    vect.push_back(hash_pair(npc,npc_data));

    rc->hmset(ss_map_ins_key.str(),vect);

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

    map_instance_id =  id ;
    map_instance_id =  map_instance_id << 32;

    std::cout << "get map_instance_id : " << map_instance_id << std::endl;

    Json::Value root;
    Json::FastWriter writer;

    root["type"] = 1;
    root["id"]   = id;
    root["first"] = 0;
    std::string task_ok = writer.write(root);
    unsigned int len = task_ok.length();
    memcpy(sendbuf,&len,sizeof(unsigned int));
    memcpy(sendbuf+sizeof(unsigned int),task_ok.c_str(),len);
    send_len = len+sizeof(unsigned int);
    psocket->async_send( boost::asio::buffer(sendbuf,send_len),boost::bind(&Generator::write_handler,this,_1,_2));

    (*this)();
}
