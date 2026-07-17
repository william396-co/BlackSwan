#include "async_accept.h"

#include <iostream>
#include <boost/asio.hpp>

void async_accept_test()
{
    using namespace boost;
	using boost::asio::ip::tcp;

    boost::asio::io_context io;

    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 9527));

    std::cout << "1 start async_accept....\n";
    acceptor.async_accept([](boost::system::error_code const &error, tcp::socket socket)
                          {
			if (error) {
				std::cerr << error.message() << "\n";
			}
			else {
				socket.send(boost::asio::buffer("reply to client:[hello client]"));
				std::cout << "4 new socket connected!\n";
			} });

    std::cout << "2 asyn_accept already return(no connected yet)\n";
    std::cout << "3. start run(), blocked here waiting for event comming.....\n";

    io.run();

    std::cout << "5 run() exit\n";
}