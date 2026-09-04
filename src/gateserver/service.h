#pragma once

#include <memory>
#include <atomic>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"
#include "share/networkEx/connector.h"

class GateService
{
public:
	GateService() = default;
	// not allow copy and move
	GateService(GateService const&) = delete;
	GateService& operator=(GateService const&) = delete;
	GateService(GateService&&) = delete;
	GateService& operator=(GateService&&) = delete;
public:
	bool start();
	void run();
	void stop();
private:
	std::atomic<bool> stop_{};
	std::shared_ptr<IoContextPool> pool_{};
	std::unique_ptr<Server> server_{};
	std::unique_ptr<Connector> server_connector_{};
	std::unique_ptr<Connector> login_connector_{};
	std::unique_ptr<boost::asio::signal_set> signals_{};
};
