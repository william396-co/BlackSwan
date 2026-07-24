#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <deque>

#include <boost/asio.hpp>

#include "packet.h"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket)
        : socket_{ std::move(socket) }
    {
        set_socket_option();   
    }

    void send(std::string msg)
    {        
        send(std::make_shared<std::string>(std::move(msg)));
    }

private:
	void send(std::shared_ptr<std::string> msg)
	{
		{
			std::lock_guard lk(sendMtx_);
			sendMsg_.push_back(std::move(msg));
		}
		trySend();
	}

	void trySend() {
		std::lock_guard lk(sendMtx_);
	}

	void set_socket_option() {
		socket_.non_blocking();
		boost::asio::ip::tcp::no_delay option(true);
		socket_.set_option(option);
	}
private:    
    tcp::socket socket_;
    std::mutex sendMtx_;
    std::deque<std::shared_ptr<std::string>> sendMsg_;
};