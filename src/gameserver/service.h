#pragma once

#include <memory>
#include <atomic>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"

class GameService
{
public:
	GameService() = default;
	// not allow copy and move
	GameService(GameService const&) = delete;
	GameService& operator=(GameService const&) = delete;
	GameService(GameService&&) = delete;
	GameService& operator=(GameService&&) = delete;
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

