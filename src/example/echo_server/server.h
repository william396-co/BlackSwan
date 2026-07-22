#pragma once

#include <boost/asio.hpp>
#include <iostream>

#include "session.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>


using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::use_awaitable;

using boost::asio::ip::tcp;


// responsble for recieve new connections
class Server {

public:
	Server(boost::asio::io_context& io, unsigned short port)
		:acceptor_{ io,tcp::endpoint(tcp::v4(),port) }
	{
		doAccept();
	}
private:
	void doAccept() {
		acceptor_.async_accept(
			[this](boost::system::error_code const& error, tcp::socket socket) {
				if (!error) {
					std::cout << "new connection: " << socket.remote_endpoint() << "\n";

					std::make_shared<Session>(std::move(socket))->start();
				}
				// whether it success or not, continue to accept next connection
				doAccept();
			});
	}
private:
	tcp::acceptor acceptor_;
};

awaitable<void> echo(tcp::socket socket);
awaitable<void> listener(tcp::acceptor acceptor);