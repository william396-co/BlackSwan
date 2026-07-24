#include "server.h"
#include <iostream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(){

    
    try{

        std::cout << "Server Start...\n";

        auto port = 9527;

        boost::asio::io_context io;
        tcp::endpoint ep(tcp::v4(), port);
        Server server(io, ep);

        server.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    std::cout << "Server end\n";
    return 0;
}