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
		:io_{io},
		connectTimer_{io},
		reconnectTimer_{io}
	{
	}

	~Connector() {
		Stop();
	}
	void asyncConnect(std::string const& host,
		uint16_t port,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback,
		std::chrono::seconds connect_timeout = std::chrono::seconds{5})
	{
		asyncConnect({ tcp::endpoint(boost::asio::ip::make_address(host), port) },  callback, failedCallback,connect_timeout);
	}

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
		auto_reconnect_.store(false, std::memory_order_release);
		connected_.store(false, std::memory_order_release);
		connectTimer_.cancel();
		reconnectTimer_.cancel();
		auto session = std::move(session_);
		if (session) {
			session->stop();
		}
	}
	void asyncConnect(std::vector<tcp::endpoint> const& eps,
		AsyncConnectCallback callback,
		AsyncConnectFailedCallback failedCallback,
		std::chrono::seconds connect_timeout = std::chrono::seconds{5})
	{
		endpoints_ = eps;
		connect_timeout_ = connect_timeout;
		connected_callback_ = std::move(callback);
		failed_callback_ = std::move(failedCallback);
		retry_delay_ = initial_retry_delay_;
		auto_reconnect_.store(true, std::memory_order_release);
		startConnect();
	}

	inline SessionPtr session() { return session_; }
private:
	void onConnected(SessionPtr connectedSession)
	{
		connected_.store(true, std::memory_order_release);
		retry_delay_ = initial_retry_delay_;
		connectedSession->SetDisconnectProc(
			[this](SessionPtr disconnectedSession) {
				if (session_ != disconnectedSession) {
					return;
				}

				connected_.store(false, std::memory_order_release);
				session_.reset();
				if (disconnect_proc_) {
					disconnect_proc_(disconnectedSession);
				}
				scheduleReconnect();
			});
		if (connected_callback_) {
			connected_callback_(connectedSession);
		}
		connectedSession->Start();
	}

	void startConnect()
	{
		if (!auto_reconnect_.load(std::memory_order_acquire) || endpoints_.empty()) {
			return;
		}

		connected_.store(false, std::memory_order_release);
		auto session = std::make_shared<Session>(io_);
		session_ = session;
		const auto attempt = ++connect_attempt_;
		auto completed = std::make_shared<std::atomic_bool>(false);

		if (connect_timeout_ > std::chrono::seconds::zero()) {
			connectTimer_.expires_after(connect_timeout_);
			connectTimer_.async_wait([this, session, completed, attempt](boost::system::error_code const& error) {
				if (error || completed->exchange(true, std::memory_order_acq_rel) || attempt != connect_attempt_) {
					return;
				}
				session->stop();
				handleConnectFailure(tcp::endpoint{});
			});
		}

		session->Connect(
			endpoints_,
			[this, completed, attempt](SessionPtr connectedSession) {
				if (completed->exchange(true, std::memory_order_acq_rel) || attempt != connect_attempt_) {
					return;
				}
				connectTimer_.cancel();
				onConnected(connectedSession);
			},
			[this, completed, attempt](tcp::endpoint ep) {
				if (completed->exchange(true, std::memory_order_acq_rel) || attempt != connect_attempt_) {
					return;
				}
				connectTimer_.cancel();
				handleConnectFailure(ep);
			});
	}

	void handleConnectFailure(tcp::endpoint ep)
	{
		connected_.store(false, std::memory_order_release);
		session_.reset();
		if (failed_callback_) {
			failed_callback_(ep);
		}
		scheduleReconnect();
	}

	void scheduleReconnect()
	{
		if (!auto_reconnect_.load(std::memory_order_acquire)) {
			return;
		}

		const auto delay = retry_delay_;
		retry_delay_ = std::min(retry_delay_ * 2, max_retry_delay_);
		reconnectTimer_.expires_after(delay);
		reconnectTimer_.async_wait([this](boost::system::error_code const& error) {
			if (!error) {
				startConnect();
			}
		});
	}
private:
	boost::asio::io_context& io_;
	SessionPtr session_{};
	DisconnectProcess disconnect_proc_;
	AsyncConnectCallback connected_callback_;
	AsyncConnectFailedCallback failed_callback_;
	std::vector<tcp::endpoint> endpoints_;
	std::chrono::seconds connect_timeout_{};
	std::chrono::seconds retry_delay_{std::chrono::seconds{1}};
	static constexpr auto initial_retry_delay_ = std::chrono::seconds{1};
	static constexpr auto max_retry_delay_ = std::chrono::seconds{30};
	boost::asio::steady_timer connectTimer_;
	boost::asio::steady_timer reconnectTimer_;
	std::atomic_bool connected_ = false;
	std::atomic_bool auto_reconnect_ = false;
	std::atomic_uint64_t connect_attempt_ = 0;
};

using ConnectorPtr = std::shared_ptr<Connector>;
