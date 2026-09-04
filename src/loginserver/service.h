#pragma once

#include <atomic>
#include <memory>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"

class LoginService 
{
public:
	LoginService() = default;
	// not allow copy and move
	LoginService(LoginService const&) = delete;
	LoginService& operator=(LoginService const&) = delete;
	LoginService(LoginService&&) = delete;
	LoginService& operator=(LoginService&&) = delete;
public:
	bool start();
	void run();
	void stop();
private:
	std::atomic<bool> stop_{};
	std::shared_ptr<IoContextPool> pool_{};
	std::unique_ptr<Server> server_{};
	std::unique_ptr<boost::asio::signal_set> signals_{};
};
