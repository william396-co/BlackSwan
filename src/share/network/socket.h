#pragma once

#include <algorithm>
#include <memory>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class SharedSocket {
public:
	SharedSocket(tcp::socket socket, boost::asio::io_context& io)
		:socket_{ std::move(socket) },
		io_{ io } 
	{
	}
	tcp::socket& socket() { return socket_; }
	boost::asio::io_context& io_context() { return io_; }
private:
	tcp::socket socket_;
	boost::asio::io_context& io_;
};
using SharedSocketPtr = std::shared_ptr<SharedSocket>;