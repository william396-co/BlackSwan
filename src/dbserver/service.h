#pragma once

#include <memory>
#include <atomic>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"
#include "share/networkEx/connector.h"

class DBService
{
public:
	DBService() = default;
	// not allow copy and move
	DBService(DBService const&) = delete;
	DBService& operator=(DBService const&) = delete;
	DBService(DBService&&) = delete;
	DBService& operator=(DBService&&) = delete;
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
