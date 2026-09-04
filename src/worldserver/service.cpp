#include "service.h"

#include <deque>
#include <exception>
#include <stdexcept>

#include "networkEx/connector.h"
#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "networkEx/session.h"
#include "proto/protocol.h"
#include "log/log.h"


#include "config.h"

constexpr auto listen_port = 8542;

bool WorldService::start()
{
	LOG_INFO("GateServer starting....");

	try {

		// Config Init
		if (!g_Config->Init()) {
			LOG_ERROR("Init Config failed");
			return false;
		}

		pool_ = std::make_shared<IoContextPool>(IoContextPool::DefaultPoolSize(),
			IoContextPool::DefaultConcurrencyHint());
		pool_->start();


		// Terminate Server SIGNAL
		signals_  =std::make_unique<boost::asio::signal_set>(pool_->getNext(), SIGINT, SIGTERM);
		signals_->async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop_.exchange(true)) {
				return;
			}

			LOG_INFO("received signal, stopping server");
			}
		);

		// server
		server_ = std::make_unique<Server>(pool_, listen_port);
		server_->start(
			[](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Session fd:{} Send GateServer PING", s->fd());
						s->sendInnerPing();
					});
				//g_gateSessionMgr->addSession(session);
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return 0; //g_packetParser->onRecvData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				//g_gateSessionMgr->delSession(session->fd());
			}
		);

		if (!stop_) {
			LOG_INFO("WorldServer running, listen port:[{}]", listen_port);
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception: {}", e.what());
		return false;
	}

	return !stop_;
}

void WorldService::run()
{
	// main thread handle
	while (!stop_.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void WorldService::stop() {

	stop_.store(true);
	server_->stop();
	pool_->stop();
}