#pragma once 

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>


#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "session.h"
#include "ioContextPool.h"

using boost::asio::ip::tcp;

class Connector
{
public:
	explicit Connector(boost::asio::io_context&io)
		:io_{io}
	{
	}

	~Connector() {
		Stop();
	}
	void asyncConnect(std::string const& host,
		uint16_t port,
		std::chrono::seconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback)
	{
		asyncConnect({ tcp::endpoint(boost::asio::ip::make_address(host), port) }, timeout, callback, failedCallback);
	}

	void Connect(std::string const& host, uint16_t port);

	bool isConnected() const {
		return connected_.load(std::memory_order_acquire);
	}
	void SetDisconnectProc(DisconnectProcess disconnect_proc) {
		disconnect_proc_ = std::move(disconnect_proc);
	}

	void send(std::string const& msg) {
		if (msg.empty()|| !isConnected()) {
			return;
		}

		auto session = session_;
		if (!session) {
			return;
		}

		session->send(msg);
	}

	void Stop() {
		connected_.store(false, std::memory_order_release);
		auto session = std::move(session_);
		if (session) {
			session->stop();
		}
	}
	void asyncConnect(std::vector<tcp::endpoint> const& eps,
		std::chrono::seconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback)
	{
		wrapperAsyncConnect(eps, timeout, callback, failedCallback);
	}
	inline SessionPtr session() { return session_; }
private:
	void onConnected(SessionPtr connectedSession, AsyncConnectCallback callback)
	{
		connected_.store(true, std::memory_order_release);
		connectedSession->SetDisconnectProc(
			[this](SessionPtr disconnectedSession) {
				connected_.store(false, std::memory_order_release);
				if (disconnect_proc_) {
					disconnect_proc_(std::move(disconnectedSession));
				}
			});
		callback(connectedSession);
		connectedSession->Start();
	}

	void wrapperAsyncConnect(std::vector<tcp::endpoint>const& eps,
		std::chrono::seconds timeout,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback)
	{
		connected_.store(false, std::memory_order_release);
		session_ = std::make_shared<Session>(io_);
		if (timeout <= std::chrono::seconds::zero()) {
			session_->Connect(
				eps,
				[this, callback = std::move(callback)](SessionPtr connectedSession) mutable {
					onConnected(std::move(connectedSession), std::move(callback));
				},
				[this, failedCallback = std::move(failedCallback)](tcp::endpoint ep) mutable {
					connected_.store(false, std::memory_order_release);
					failedCallback(ep);
				});
			return;
		}

		auto timer = std::make_shared<boost::asio::steady_timer>(io_, timeout);
		auto completed = std::make_shared<std::atomic_bool>(false);
		auto session = session_;

		timer->async_wait([session, completed, failedCallback](boost::system::error_code const& error) {
			if (error || completed->exchange(true, std::memory_order_acq_rel)) {
				return;
			}

			session->stop();
			failedCallback(tcp::endpoint{});
		});

		session_->Connect(
			eps,
			[this, callback = std::move(callback), timer, completed](SessionPtr connectedSession) mutable {
				if (completed->exchange(true, std::memory_order_acq_rel)) {
					return;
				}

				timer->cancel();
				onConnected(std::move(connectedSession), std::move(callback));
			},
			[this, failedCallback = std::move(failedCallback), timer, completed](tcp::endpoint ep) mutable {
				if (completed->exchange(true, std::memory_order_acq_rel)) {
					return;
				}

				connected_.store(false, std::memory_order_release);
				timer->cancel();
				failedCallback(ep);
			});
	}
private:
	boost::asio::io_context& io_;
	SessionPtr session_{};
	DisconnectProcess disconnect_proc_;
	std::atomic_bool connected_ = false;
};

using ConnectorPtr = std::shared_ptr<Connector>;