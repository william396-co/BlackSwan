#include "service.h"

#include <memory>
#include <atomic>
#include <deque>
#include <exception>
#include <stdexcept>

#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"
#include "share/log/log.h"

#include "player.h"
#include "packetParser.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "config.h"


constexpr auto listen_port = 8321;
constexpr auto port = static_cast<boost::asio::ip::port_type>(listen_port);

bool GameService::start() 
{

	LOG_INFO("GameService starting....");

	try
	{	

		if (!g_Config->Init()) {
			LOG_ERROR("config init failed");
			return false;
		}

		// io running pool 
		pool_ = std::make_shared<IoContextPool>(
			IoContextPool::DefaultPoolSize(),
			IoContextPool::DefaultConcurrencyHint());
		pool_->start();


		// server
		server_ = std::make_unique<Server>(pool_, port);
		server_->start(
			[](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Session fd:{} Send GateServer PING", s->fd());
						s->sendInnerPing();
					});
				g_gateSessionMgr->addSession(session);
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return g_packetParser->onRecvData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				g_gateSessionMgr->delSession(session->fd());
			}
		);

		signals_ = std::make_unique<boost::asio::signal_set>(pool_->getNext(), SIGINT, SIGTERM);
		signals_->async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop_.exchange(true)) {
				return;
			}

			LOG_INFO("received signal, stopping server");
			}
		);

		if (!stop_) {
			LOG_INFO("GameServer running, listen port:[{}]", listen_port);

			// packetParser Init
			g_packetParser->Init();
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception:{} ", e.what());
		return false;
	}
	return true;
}

void GameService::run()
{
	// main thread handle
	while (!stop_.load())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
void GameService::stop()
{
	stop_.store(true);
	// IO Level
	server_->stop();
	pool_->stop();
}