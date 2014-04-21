#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <redisclient.h>
#include <string>
#include <vector>




class Generator:boost::asio::coroutine{
public:
    Generator(
        boost::asio::io_service& _io_service,
        const std::string address,
        int port,
        std::string redishost,
        int redisport,
        int _dur_time=200
        );
    void operator()(
        boost::system::error_code ec = boost::system::error_code(),
        std::size_t length = 0
    );
    void HandleTask(std::string task);
    void write_handler(boost::system::error_code ec,std::size_t);
    void is_over_time(int gen_duration,int map_type_id);
    void creatMap(int type,int num);
    void sendMap(std::string & rmap,std::string & npc_data, int type);
    void clearbuf();
    void set_dur_time(int durtime);
    void reconnect();
protected:
private:
    typedef boost::asio::ip::tcp tcp;
    int dur_time;
    int id;
    boost::uint64_t map_instance_id;
    char  readbuf[1024];
    char  sendbuf[1024];
    unsigned int send_len;
    unsigned int read_len;
    boost::asio::io_service & io_service;
    boost::shared_ptr<redis::client> rc;
    boost::shared_ptr<tcp::socket> psocket;
    tcp::endpoint endpoint;
};
