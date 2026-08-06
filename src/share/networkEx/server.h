#pragma  once

#include <iostream>
#include <atomic>
#include <mutex>
#include <deque>
#include <memory>
#include <thread>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "session.h"
#include "ioContextPool.h"


using boost::asio::awaitable;
using boost::asio::ip::tcp;
using boost::asio::use_awaitable;


using AcceptHandle = std::function<void(SessionPtr)>;

class Server
{
public:
	Server(IoContextPoolPtr pool, uint16_t port)
		:pool_{pool},
		acceptor_(pool->getNext(), tcp::endpoint(tcp::v4(), port))
	{

	}
	~Server() {
		stop();
	}

	void start(AcceptHandle handle,DataProcess decode_proc,DisconnectProcess disconnect_proc)
	{
		accept_handle_ = std::move(handle);
		data_proc_ = std::move(decode_proc);
		disconnect_proc_ = std::move(disconnect_proc);

		boost::asio::co_spawn(
			acceptor_.get_executor(),
			[this]() { return listener();	},
			boost::asio::detached);
	}
	void stop() {
		boost::system::error_code ignored;
		acceptor_.cancel(ignored);
		acceptor_.close(ignored);
		closeAll();
	}
	void broadcast(std::string const& msg) {
		SessionMap curList;
		{
			std::lock_guard lk(sessions_mtx_);
			curList = sessions_;
		}
		for (auto& it : curList) {
			it.second->send(msg);
		}
	}
	void send(std::string const& msg, SessionPtr s) {
		s->send(msg);
	}
	SessionPtr FindSession(uint32_t fd) {
		std::lock_guard lk(sessions_mtx_);
		return sessions_[fd];
	}
private:
	awaitable<void> listener() {

		for (;;) {

			boost::system::error_code error;
			auto& session_io = pool_->getNext();
			auto socket = co_await acceptor_.async_accept(session_io,boost::asio::redirect_error(use_awaitable, error));
			if (error == boost::asio::error::operation_aborted) {
				std::cout << "[system] listener stopped\n";
				co_return;
			}
			if (error) {
				std::cerr << "[system] accept failed: " << error.message() << "\n";
				continue;
			}
			std::cout << "[system] new connection: " << socket.remote_endpoint() << "\n";

			auto session = std::make_shared<Session>(std::move(socket));
			session->SetDisconnectProc(
				[this](auto s) {
					disconnect_proc_(s);
					delSession(s);
				}
			); 
			session->SetDataProc(data_proc_);
			accept_handle_(session);
			addSession(session);
			session->Start();
		}
	}

	void addSession(SessionPtr s) {
		std::lock_guard lk(sessions_mtx_);
		sessions_.emplace(s->fd(), s);
	}
	void delSession(SessionPtr s) {
		std::lock_guard lk(sessions_mtx_);
		sessions_.erase(s->fd());
	}
	void closeAll(){
		SessionMap curList;
		{
			std::lock_guard lk(sessions_mtx_);
			curList = std::move(sessions_);
		}
		for(auto&it: curList){
			it.second->stop();
		}
	}
private:
	AcceptHandle accept_handle_;
	DisconnectProcess disconnect_proc_;
	DataProcess data_proc_;
	IoContextPoolPtr pool_;
	tcp::acceptor acceptor_;
	SessionMap sessions_;
	std::mutex sessions_mtx_;
};
