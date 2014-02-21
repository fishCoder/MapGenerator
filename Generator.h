#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <redisclient.h>
#include <string>
#include <vector>

#include "FieldMapGenerator.h"



class Generator:boost::asio::coroutine{
public:
    Generator(
        boost::asio::io_service& io_service,
        const std::string address,
        int port);
    void operator()(
        boost::system::error_code ec = boost::system::error_code(),
        std::size_t length = 0
    );
    void HandleTask(std::string task);
    void creatMap(int type,int num);
    void sendMap(std::string & rmap, int type);
    void clearbuf();
    void reconnect();
protected:
private:
    typedef boost::asio::ip::tcp tcp;
    std::string task_ok;
    int id;
    char * readbuf;
    boost::shared_ptr<redis::client> rc;
    boost::shared_ptr<tcp::socket> psocket;
    FieldMapGenerator fieldmapGenerator;
};
