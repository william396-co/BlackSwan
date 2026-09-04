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

#include "packetParser.h"
#include "player.h"
#include "playerCtrl.h"
#include "clientSession.h"
#include "clientSessionMgr.h"
#include "config.h"

uint16_t gate_port = 9527;
constexpr auto game_port = 8321;
constexpr auto login_port = 8600;
constexpr auto host = "127.0.0.1";

bool GateService::start() 
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


		// server connector to GameSever
		server_connector_ = std::make_unique<Connector>(pool_->getNext());
		server_connector_->SetDisconnectProc([](SessionPtr) {
			LOG_ERROR("GameServer Connector disconnected");
			});

		// Async Connector to GameSever
		server_connector_->asyncConnect(host, game_port,
			[](SessionPtr session) {
				std::cout << "Connect successed:" << session->remote_ep() << "\n";
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Session fd: {} Send GameServer PING", s->fd());
						s->sendInnerPing();
					});
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {
					return g_packetParser->onRecvServerData(data, len, session);
					});
			},
			[](tcp::endpoint ep) {
				//LOG_ERROR("Connect GameServer {} failed", ep);
			}
		);


		// loginServer connector to LoginServer
		login_connector_ = std::make_unique<Connector>(pool_->getNext());
		login_connector_->SetDisconnectProc([](SessionPtr) {
			LOG_ERROR("LoginServer Connector disconnected");
			});

		// Async Connector to LoginServer
		login_connector_->asyncConnect(host, login_port,
			[](SessionPtr session) {
				//LOG_INFO("Connect successed:{}", session->remote_ep());
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Session fd:{}  Send LoginServer PING", s->fd());
						s->sendInnerPing();
					});
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {
					return g_packetParser->onRecvLoginData(data, len, session);
					});
			},
			[](tcp::endpoint ep) {
				//LOG_ERROR("Connect LoginServer {} failed", ep);
			}
		);

		// GateServer Listen Client Connect
		server_ = std::make_unique<Server>(pool_, gate_port);
		server_->start(
			[game_conn = server_connector_.get(), login_conn = login_connector_.get()](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Session fd: {} Send Client PING", s->fd());
						s->sendPing();
					});
				g_clientSessionMgr->addSession(session, game_conn, login_conn);
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return g_packetParser->onRecvClientData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				g_clientSessionMgr->delSession(session);
			}
		);

		// GameServer already start Service
		if (server_connector_->isConnected() && login_connector_->isConnected()) {
			LOG_INFO("GateServer running, listen port:{}", gate_port);
			stop_.store(false, std::memory_order_acquire);
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception: {}", e.what());
		return false;
	}

	return !stop_;
}

void GateService::run() 
{
	// main thread handle
	while (!stop_.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void GateService::stop() {

	stop_.store(true);
	server_->stop();
	login_connector_->Stop();
	server_connector_->Stop();
	pool_->stop();
}