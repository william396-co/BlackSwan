#include "server.h"
#include "session.h"

#include <iostream>

Server::Server(boost::asio::io_context& io, tcp::endpoint const& ep)
	:io_{io},
	acceptor_(io_,ep)
{	
	start();
}

Server::~Server()
{	
	io_.stop();
}

void Server::callback_session(std::string const& fromIp, std::string const& info)
{
	std::cout << "[" << fromIp << "]:  " << info << "\n";
}

void Server::accept_handler(SessionPtr session, boost::system::error_code const& error)
{
	if (!error && session) {
		try {
			session->start();
			start();
		}
		catch (...) {

		}
	}
}

void Server::start() {

	auto session = std::make_shared<Session>(io_);

	acceptor_.async_accept(session->socket(),
		std::bind(&Server::accept_handler, this, session, std::placeholders::_1));
}

void Server::run() {
	io_.run();
}