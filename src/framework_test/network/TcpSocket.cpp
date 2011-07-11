// TcpSocket.cpp

#include "framework_test/Common.h"

#include <framework/network/TcpSocket.h>
using namespace framework::system;
using namespace framework::configure;
using namespace framework::network;

#include <boost/thread/thread.hpp>

static void test_network_tcp_socket(Config & conf)
{
    NetName addr("192.168.1.109", 1802);
    std::string name;

    conf.register_module("TestTcpSocket")
        << CONFIG_PARAM_RDWR(addr)
        << CONFIG_PARAM_RDWR(name);

    boost::system::error_code ec;
    boost::asio::io_service io_svc;
    TcpSocket socket(io_svc);
    socket.set_non_block(true, ec);
    socket.connect(addr, ec);
    while (ec == boost::asio::error::would_block) {
        std::cout << "connect:" << ec.message() << std::endl;
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        socket.connect(addr, ec);
    };
    std::cout << "connect:" << ec.message() << std::endl;
}

static TestRegister test("network_tcp_socket", test_network_tcp_socket);
