#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <array>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {

public:
	explicit Session(tcp::socket socket)
		:socket_{ std::move(socket) } {
	}

	void start() {
		doRead();
	}
private:
	void doRead() {

		socket_.async_read_some(
			boost::asio::buffer(data_, sizeof(data_)),
			[this, self = shared_from_this()](boost::system::error_code const& error, size_t len) {

				if (!error) {					
					std::cout << "received data:[" << data_.data() << "]\n";					
					doWrite(len);//echo ,send data back
				}
				else {
					std::cerr << error.what() << "\n";
				}
			}
		);

	}
	void doWrite(size_t len) {

		boost::asio::async_write(
			socket_, boost::asio::buffer(data_, len),
			[this, self = shared_from_this()](boost::system::error_code const& error, size_t) {
				if (!error) {
					std::cout << "send data:[" << data_.data() << "]\n";
					doRead();
				}
				else {
					std::cerr << error.what() << "\n";
				}
			}
		);
	}
private:
	tcp::socket socket_;
	std::array<char, 1024> data_{};
};
