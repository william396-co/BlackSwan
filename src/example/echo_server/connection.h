#pragma once

#include <memory>
#include <vector>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>

using boost::asio::ip::tcp;

class Connection : public std::enable_shared_from_this<Connection> {
public:
	Connection(boost::asio::io_context& io, tcp::socket socket)
		:strand_{ boost::asio::make_strand(io) }
		, socket_{ std::move(socket) }
	{

	}

	void send(std::string const& msg) {
		boost::asio::post(strand_,
			[self = shared_from_this(), msg]() {
				self->doSend(msg);
			}
	}
private:
	void doRead() {
	   // bind_executor：将 handler 绑定到 strand
	   // 效果：读操作完成时，handler 一定在 strand 中执行
		socket_.async_read_some(
			boost::asio::buffer(readBuf_),
			boost::asio::bind_executtor(stdrand_,
				[self = shared_from_this()]
				(boost::system::error_code const& error, size_t n) {
					if (!error) {
						self->processData(n);
						self->doRead();
					}
				}
			)
		);
	}
	void doSend(std::string const& msg) { 
		// 此函数一定在 strand 中执行
		// 可以安全访问 writeQueue_ 而无需加锁
		writeQueue_.push_back(msg);
		if (writeQueue_.size() == 1) {
			doWrite();
		}
	}

	void doWrite();
	void processData(size_t n);
private:
	std::vector<std::string> writeQueue_;
	char readBuf_[1024 * 4];
	tcp::socket socket_;
	boost::asio::strand<boost::asio::io_context::executor_type> starand_;
};