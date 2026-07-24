#pragma once

#include <memory>
#include <chrono>
#include <functional>

#include <boost/asio.hpp>

class WrapperIoService{
public:
	using Ptr = std::shared_ptr<WrapperIoService>;

	WrapperIoService(int concurrencyHint)
		:tricky_io_ctx_(std::make_shared<boost::asio::io_context>(concurrencyHint))
		, io_ctx_(*tricky_io_ctx_)
	{
	}

	WrapperIoService(boost::asio::io_context& io)
		:io_ctx_{ io } 
	{
	}

	void run() {				
		while (!io_ctx_.stopped()) {
			io_ctx_.run();
		}
	}

	void stop() {
		io_ctx_.stop();
	}

	boost::asio::io_context& io_context() {
		return io_ctx_;
	}

	auto runAfter(std::chrono::nanoseconds timeout, std::function<void()> callback) {
		auto timer = std::make_shared<boost::asio::steady_timer>(io_ctx_);
		timer->expires_after(timeout);
		timer->async_wait([timer, callback](boost::system::error_code const&error) {
			if (!error) {
				callback();
			}
			});
		return timer;
	}
private:
	std::shared_ptr<boost::asio::io_context> tricky_io_ctx_;
	boost::asio::io_context& io_ctx_;
};