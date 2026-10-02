#include "service.h"

#include <memory>
#include <atomic>


#include "networkEx/connector.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"
#include "share/log/log.h"
#include "utils/runningFlag.h"

#include "player.h"
#include "packetParser.h"
#include "config.h"
#include "playerMgr.h"

auto host = "127.0.0.1";
uint16_t port = 9527;

extern std::atomic<bool> g_stop_flag_;

bool ClientService::start()
{
	LOG_INFO("ClientService starting....");

	try {

		// Config Init
		if (!g_Config->Init()) {
			LOG_ERROR("Init Config failed");
			return false;
		}

		pool_ = std::make_shared<IoContextPool>(IoContextPool::DefaultPoolSize(),
			IoContextPool::DefaultConcurrencyHint());
		pool_->start();

		// connector 
		connector_ = std::make_unique<Connector>(pool_->getNext());
		connector_->setDisconnectProc([this](SessionPtr) {
			disableGameRunning();
			LOG_ERROR("disconnected from GateServer");
			}
		);

		connector_->asyncConnect(host, port,
			[this](SessionPtr session) {
				LOG_INFO("connect successed: {}:{}", session->remote_ep().address().to_string(), std::to_string(session->remote_ep().port()));
				enableGameRunning();
				session->startHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Client fd:{} Send GateServer PING", s->fd());
						s->sendPing();
					});
				session->setDataProc([](const char* data, size_t len, SessionPtr session)->size_t {// decode call back					
					return g_packetParser->onRecvData(data, len, session);
					});
#if 0
				auto pPlayer = findPlayer(1);
				// send LoginReq
				pPlayer->sendLoginReq();
#endif
			},
			[](tcp::endpoint ep) {
				LOG_ERROR("connect {}:{} failed", ep.address().to_string(), std::to_string(ep.port()));
			}
		);


		// elegant close io_context
		signals_ = std::make_unique<boost::asio::signal_set>(pool_->getNext(), SIGINT, SIGTERM);
		signals_->async_wait([&](boost::system::error_code const& error, int) {
			if (error || g_stop_flag_.exchange(true)) {
				return;
			}

			LOG_INFO("received signal, stopping client");
			}
		);

		if (!isGameRunning()) {
			LOG_INFO("ClientService running....");

			g_packetParser->Init();
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception: {}", e.what());
		disableGameRunning();
		return false;
	}

	return isGameRunning();
}

void ClientService::run()
{
	// main thread handle
	std::string input;	
	while (isGameRunning())
	{
		// send message in main_thread
		if (!(std::cin >> input)) {
			disableGameRunning();
			break;
		}
		if (!isGameRunning() || !connector_->isConnected()) {
			break;
		}		
		connector_->send(encode_packet((uint32_t)MsgId::ECHO_REQ, input.c_str(), (uint16_t)input.size()));
		input.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
	}
}

void ClientService::stop() {

	disableGameRunning();
	connector_->stop();
	pool_->stop();
}