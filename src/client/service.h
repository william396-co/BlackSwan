#pragma once

#include <memory>
#include <atomic>

#include "share/networkEx/server.h"
#include "share/networkEx/ioContextPool.h"
#include "share/networkEx/connector.h"

class ClientService
{
public:
	ClientService() = default;
	// not allow copy and move
	ClientService(ClientService const&) = delete;
	ClientService& operator=(ClientService const&) = delete;
	ClientService(ClientService&&) = delete;
	ClientService& operator=(ClientService&&) = delete;
public:
	bool start();
	void run();
	void stop();

	Connector* getConn() { return connector_ ? connector_.get() : nullptr; }
private:
	std::atomic<bool> stop_{};
	std::shared_ptr<IoContextPool> pool_{};
	std::unique_ptr<Connector> connector_{};
	std::unique_ptr<boost::asio::signal_set> signals_{};
};
