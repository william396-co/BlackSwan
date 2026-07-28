#pragma once
#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <memory>
#include <deque>
#include <functional>

#include "packet.h"

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::use_awaitable;

using DataProcess = std::function<size_t(const char*, size_t)>;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
	explicit ChatSession(boost::asio::io_context& io_context)		
		:socket_{ io_context },
		writeTimer_{ io_context }
	{
	}
	ChatSession(tcp::socket socket)
		: socket_{ std::move(socket) }
		, writeTimer_{ socket_.get_executor() } 
	{
	}
	
	// for Client Session
	void Connect(std::string const& host, std::string const& port) {

		tcp::resolver resolver(socket_.get_executor());
		tcp::resolver::results_type ep = resolver.resolve(host, port);

		boost::asio::async_connect(socket_, ep,
			[self = shared_from_this()](boost::system::error_code const& error, tcp::endpoint remote_ep) {
				if (!error) {
					self->Start();// after connect ,then start read/write coroutine
				}
				else {
					std::cerr << "connect" << remote_ep << " failed:[" << error.message() << "]\n";
				}
			});
	}
	~ChatSession() {
		if (socket_.is_open()) {
			std::cout << "[" << socket_.remote_endpoint() << "]" << __func__ << "\n";
		}
		stop();
	}
	// for Server Start Session(start read/write coroutine)
	void Start() {
		// active write and read coroutine
		boost::asio::co_spawn(
			socket_.get_executor(),
			[self = shared_from_this()]() { return	self->readerLoop();	},
			boost::asio::detached);

		boost::asio::co_spawn(
			socket_.get_executor(),
			[self = shared_from_this()]() { return	self->writerLoop();	},
			boost::asio::detached);
	}
	void send(std::string const& msg) {
		deliver(msg);
	}
	void SetDataProc(DataProcess data_proc) { data_proc_ = data_proc; }	
private:
	void deliver(std::string const& msg) {
		sendQueue_.push_back(encode_packet(1, msg.c_str(), msg.size()));
		// wake up write-coroutine by cancel timer once
		writeTimer_.cancel_one();
	}
	awaitable<void> readerLoop() {

		try {

			//boost::asio::streambuf buf;
			char buf[1024 * 4];
			for (;;) {

				std::fill(std::begin(buf), std::end(buf), '\0');
				auto n = co_await socket_.async_read_some(boost::asio::buffer(buf, sizeof(buf)), use_awaitable);
				if (data_proc_ && n) {
					n = data_proc_(buf, n);
				}
				// TODO adjust recvBuf
			}
		}
		catch (boost::system::error_code const& error) {
			// 在这里统一处理所有断开情况
			if (error == boost::asio::error::eof) {
				std::cout << "connection disconnected:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::connection_reset) {
				std::cout << "connection reset:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::broken_pipe) {
				std::cout << "connection broken pipe:" << socket_.remote_endpoint() << "\n";
			}
			else {
				std::cout << "other network error:" << error.message() << "\n";
			}
			// 不需要手动 close，socket 析构时会自动关闭
		}
	}

	awaitable<void> writerLoop() {

		try {

			while (socket_.is_open()) {

				// 设置一个"永不到期"的定时器作为通知机制
				// 当有新消息时，deliver() 会 cancel_one() 来唤醒这个等待
				if (sendQueue_.empty()) {

					writeTimer_.expires_at(boost::asio::steady_timer::time_point::max());

					// redirect_error：将异常转为 error_code
					// 这样 cancel 不会抛异常，而是返回 operation_aborted
					boost::system::error_code error;
					co_await writeTimer_.async_wait(boost::asio::redirect_error(use_awaitable, error));
					// ec == operation_aborted 表示被 deliver() 唤醒
				}

				// send the message in the queue
				while (!sendQueue_.empty())
				{					
					co_await boost::asio::async_write(
						socket_,
						boost::asio::buffer(sendQueue_.front()), use_awaitable);
					sendQueue_.pop_front();
				}
			}

		}
		catch (boost::system::error_code const& error) {
			// 在这里统一处理所有断开情况
			if (error == boost::asio::error::eof) {
				std::cout << "connection disconnected:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::connection_reset) {
				std::cout << "connection reset:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::broken_pipe) {
				std::cout << "connection broken pipe:" << socket_.remote_endpoint() << "\n";
			}
			else {
				std::cout << "other network error:" << error.message() << "\n";
			}
			// 不需要手动 close，socket 析构时会自动关闭
		}
	}
	void stop() {
		boost::system::error_code error;
		socket_.close(error);
		writeTimer_.cancel();
	}
private:
	tcp::socket socket_;
	boost::asio::steady_timer writeTimer_;
	std::deque<std::string> sendQueue_;
	std::deque<std::string> recvQueue_;
	DataProcess data_proc_;
};

using ChatSessionPtr = std::shared_ptr<ChatSession>;
