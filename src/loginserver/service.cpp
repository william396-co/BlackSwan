#include "service.h"

#include <stdexcept>
#include <exception>


#include <google/protobuf/stubs/common.h>

#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"
#include "log/log.h"
#include "utils/runningFlag.h"


#include "player.h"
#include "playerMgr.h"
#include "packetParser.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "apMgr.h"
#include "apMgrHandler.h"
#include "config.h"

constexpr auto listen_port = 8601;

extern std::atomic<bool> g_stop_flag_;
bool LoginService::start()
{
	LOG_INFO("LoginServer starting....");	
	constexpr auto port = static_cast<boost::asio::ip::port_type>(listen_port);

	try {

		// Config Init
		if (!g_Config->Init()) {
			LOG_ERROR("Init Config failed");
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
						LOG_DEBUG("Session fd {} Send LoginServer PING", s->fd());
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

		if (!g_apmgrhandler->start()) {
			LOG_ERROR("APMgrHandler start failed");
			return false;
		}

		signals_ = std::make_unique<boost::asio::signal_set>(pool_->getNext(), SIGINT, SIGTERM);
		signals_->async_wait([&](boost::system::error_code const& error, int) {
			if (error || g_stop_flag_.exchange(true)) {
				return;
			}

			LOG_INFO("received signal, stopping LoginServer");
			}
		);


		if (isGameRunning()) {
			LOG_INFO("LoginServer running, listen port:[{}]", listen_port);

			// packetParser Init
			g_packetParser->Init();
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception:{} ", e.what());		
		disableGameRunning();
		return false;
	}

	return isGameRunning();
}

void LoginService::run()
{
	// main thread handle
	while (isGameRunning())
	{
		g_playerMgr->onUpdate();
		g_packetParser->onUpdate();
		g_apmgr->onUpdate();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));// avoid busy loop
	}
}

void LoginService::stop()
{
	g_apmgrhandler->stop();
	disableGameRunning();
	// IO Level
	server_->stop();
	pool_->stop();
}
