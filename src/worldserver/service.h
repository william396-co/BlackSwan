#pragma once

#include <memory>
#include <atomic>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"
#include "share/networkEx/connector.h"

class WorldService
{
public:
	WorldService() = default;
	// not allow copy and move
	WorldService(WorldService const&) = delete;
	WorldService& operator=(WorldService const&) = delete;
	WorldService(WorldService&&) = delete;
	WorldService& operator=(WorldService&&) = delete;
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
