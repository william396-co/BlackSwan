#pragma once
#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <memory>
#include <deque>
#include <vector>
#include <functional>
#include <unordered_map>

#include "packet.h"

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::use_awaitable;

class Session;
using SessionPtr = std::shared_ptr<Session>;
using SessionWeakPtr = std::weak_ptr<Session>;

using DataProcess = std::function<size_t(const char*, size_t,SessionPtr)>;
using DisconnectProcess = std::function<void(SessionPtr)>;


using AsyncConnectCallback = std::function<void(SessionPtr)>;
using AsyncConnectFailedCallback = std::function<void(tcp::endpoint)>;


using SessionMap = std::unordered_map<uint32_t, SessionPtr>;

class Session : public std::enable_shared_from_this<Session>
{
public:
	explicit Session(boost::asio::io_context& io_context)
		:fd_{ id_seed_.fetch_add(1, std::memory_order_relaxed) + 1 },
		socket_{ io_context },
		writeTimer_{ io_context }
	{
		std::cout << __func__ << "(" << fd_ << ")\n";
	}
	explicit Session(tcp::socket socket)
		:fd_{ id_seed_.fetch_add(1, std::memory_order_relaxed) + 1 },
		socket_{ std::move(socket) }
		, writeTimer_{ socket_.get_executor() }
	{		
		std::cout << __func__ << "(" << fd_ << ")\n";
	}	
	// for Client Session
	void Connect(std::string const& host, uint16_t port, AsyncConnectCallback callback, AsyncConnectFailedCallback failedCallback)
	{
		Connect({ tcp::endpoint(boost::asio::ip::make_address(host), port) },callback,failedCallback);
	}
	void Connect(std::vector<tcp::endpoint>const& eps, AsyncConnectCallback callback, AsyncConnectFailedCallback failedCallback)
	{
		// Start() is intentionally explicit so callers can install callbacks first.
		boost::asio::async_connect(socket_, eps,
			[=,self = shared_from_this()](boost::system::error_code const& error, tcp::endpoint remote_ep) {
				if (!error) {
					callback(self);
				}
				else
				{
					failedCallback(remote_ep);
				}
			});
	}
	~Session() {
		std::cout << __func__ << "(" << fd_ << ")\n";
		if (socket_.is_open()) {
			std::cout << "[" << peerName() << "]" << __func__ << "\n";
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
	void send(std::string msg) {
		auto self = shared_from_this();
		boost::asio::post(socket_.get_executor(),
			[self, msg = std::move(msg)]() mutable {
				self->deliver(std::move(msg));
			});
	}
	void SetDataProc(DataProcess data_proc) { data_proc_ = std::move(data_proc); }
	void SetDisconnectProc(DisconnectProcess disconnect_proc) { disconnect_proc_ = std::move(disconnect_proc); }

	void stop() {
		if (stopped_.exchange(true)) {
			return;
		}

		auto const peer = peerName();
		std::cout << "stop [" << peer << "]\n";

		boost::system::error_code error;
		socket_.shutdown(tcp::socket::shutdown_both, error);
		socket_.close(error);
		writeTimer_.cancel();

		if (disconnect_proc_) {// process after disconnected
			disconnect_proc_(shared_from_this());
		}
	}

	tcp::endpoint remote_ep()const { return socket_.remote_endpoint(); }
	uint32_t fd()const { return fd_; }
private:
	std::string peerName() const {
		boost::system::error_code error;
		auto endpoint = socket_.remote_endpoint(error);
		if (error) {
			return "<disconnected>";
		}

		return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
	}

	void handleSocketError(boost::system::error_code const& error) {
		auto const peer = peerName();
		if (error == boost::asio::error::operation_aborted) {
			return;
		}
		if (error == boost::asio::error::eof) {
			std::cout << "connection disconnected: " << peer << "\n";
		}
		else if (error == boost::asio::error::connection_reset) {
			std::cout << "connection reset: " << peer << "\n";
		}
		else if (error == boost::asio::error::broken_pipe) {
			std::cout << "connection broken pipe: " << peer << "\n";
		}
		else {
			std::cout << "other network error: " << peer << " [" << error.message() << "]\n";
		}

		stop();
	}

	void deliver(std::string msg) {
		if (stopped_.load(std::memory_order_relaxed)) {
			return;
		}

		sendQueue_.push_back(encode_packet(1, msg.c_str(), (uint32_t)msg.size()));
		// wake up write-coroutine by cancel timer once
		writeTimer_.cancel_one();
	}
	awaitable<void> readerLoop() {

		constexpr auto BUF_SIZE = 1024 * 4;
		try {
			
			char buf[BUF_SIZE];// temporary buf for once async_read
			for (;;) {

				std::fill(std::begin(buf), std::end(buf), '\0');
				auto n = co_await socket_.async_read_some(boost::asio::buffer(buf, sizeof(buf)), use_awaitable);
				recvBuffer_.append(buf, n);
				if (!data_proc_) {
					recvBuffer_.clear();
					continue;
				}

				auto before = recvBuffer_.size();
				auto remaining = std::min(data_proc_(recvBuffer_.data(), recvBuffer_.size(), shared_from_this()), before);
				auto const consumed = recvBuffer_.size() - remaining;
				if (consumed > 0) {
					recvBuffer_.erase(0, consumed);
				}
			}
		}
		catch (boost::system::system_error const& error) {
			handleSocketError(error.code());
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
		catch (boost::system::system_error const& error) {
			handleSocketError(error.code());
		}
	}
private:
	uint32_t fd_;
	tcp::socket socket_;
	boost::asio::steady_timer writeTimer_;
	std::deque<std::string> sendQueue_;
	std::string recvBuffer_;
	DataProcess data_proc_;
	DisconnectProcess disconnect_proc_;
	std::atomic_bool stopped_ = false;
	static std::atomic_uint32_t id_seed_;
};

