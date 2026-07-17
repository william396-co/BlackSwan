#include "steady_timer.h"

#include <boost/asio.hpp>
#include <iostream>

void steady_timer_test()
{
    using namespace std::chrono_literals;

    boost::asio::io_context io;

    // create a timer after 2 second on time
    boost::asio::steady_timer timer(io, 2s);

    std::cout << "timer already setting ,waiting for 2 second....\n";

    // async wait timer on time execuate lamda
    timer.async_wait([](boost::system::error_code const &error)
                     {
			if (!error) {
				std::cout << "timer is on time\n";
			}
			else if(error==boost::asio::error::operation_aborted){
				std::cout << "timer was cancelled\n";
			} });

    io.run();
}
