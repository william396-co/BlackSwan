#include "session.h"

#include <algorithm>
#include <utility>
#include <sstream>
#include <iostream>

#include "server.h"

Session::Session(boost::asio::io_context& io)
	:socket_{io}
{
	std::fill(std::begin(msg_), std::end(msg_), '\0');
}

void Session::start() {
		
	std::string msg = "001:Connect Succeed! Please tell me with 10 bytes, the total data and the size of each package, example:128 1024";
	boost::asio::async_write(socket_, boost::asio::buffer(msg),
		std::bind(&Session::init_handler, shared_from_this(), std::placeholders::_1));
}

void Session::init_handler(boost::system::error_code const& error)
{
	if (error) {
		return;
	}
	boost::asio::async_read(socket_, boost::asio::buffer(msg_, 10),
		std::bind(&Session::analysis_handler, shared_from_this(), std::placeholders::_1));
}

void Session::analysis_handler(boost::system::error_code const& error)
{
	if (error)
		return;

	bool bflag = true;
	if (!bflag) {
		start();
		return;
	}
	
	std::stringstream is(msg_);
	is >> maxSize_;
	is >> sumSize_;

	char msg[REPLY_SIZE];
	sprintf_s(msg, "001:is ok, data remaining %d.", sumSize_);
	boost::asio::async_write(socket_, boost::asio::buffer(msg, REPLY_SIZE),
		std::bind(&Session::after_write_handler, shared_from_this(), std::placeholders::_1));
}

void Session::done_handler(boost::system::error_code const& error)
{
	if (error)return;

	currMsg_ += msg_;

	if (currMsg_.empty() && callback_) {
		callback_(socket_.remote_endpoint().address().to_string(), currMsg_);
		currMsg_.clear();
	}
	std::fill(std::begin(msg_), std::end(msg_), '\0');
	std::string msg = "001:will done.";
	boost::asio::async_write(socket_, boost::asio::buffer(msg, msg.size()),
		std::bind(&Session::init_handler, shared_from_this(),std::placeholders::_1));
}

void Session::after_read_handler(boost::system::error_code const& error, size_t readsize)
{
	if (error)return;

	currMsg_ += msg_;
	if (currMsg_.size() > 1024 * 512) {
		if (callback_) {
			callback_(socket_.remote_endpoint().address().to_string(), currMsg_);
			currMsg_.clear();
		}
	}
	std::fill(std::begin(msg_), std::end(msg_), '\0');

	sumSize_ -= readsize;

	if (sumSize_ < 0) {
		done_handler(error);
	}
	else {
		char msg[REPLY_SIZE];
		sprintf_s(msg, "001:%d.", sumSize_);
		boost::asio::async_write(socket_, boost::asio::buffer(msg, REPLY_SIZE),
			std::bind(&Session::after_write_handler, shared_from_this(),std::placeholders::_1));

		std::cout << "send client recv succeed: " << msg << std::endl;
	}

}

void Session::after_write_handler(boost::system::error_code const& error)
{
	if (error)return;

	boost::asio::async_read(socket_, boost::asio::buffer(msg_, maxSize_),
		std::bind(&Session::after_read_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}
