#include "service.h"

#include <memory>
#include <atomic>


#include "networkEx/connector.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"
#include "share/log/log.h"

#include "player.h"
#include "packetParser.h"
#include "config.h"

auto host = "127.0.0.1";
uint16_t port = 9527;

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
		connector_->SetDisconnectProc([this](SessionPtr) {
			stop_.store(true, std::memory_order_release);
			LOG_ERROR("disconnected from server");
			}
		);

		connector_->asyncConnect(host, port,
			[this](SessionPtr session) {
				//LOG_INFO("connect successed:{}", session->remote_ep());
				stop_.store(false, std::memory_order_acquire);
				session->StartHeartbeat(
					[](SessionPtr s) {
						LOG_DEBUG("Client fd:{} Send GateServer PING", s->fd());
						s->sendPing();
					});
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {// decode call back					
					return g_packetParser->onRecvData(data, len, session);
					});
			},
			[](tcp::endpoint ep) {
				//LOG_ERROR("connect {} failed", ep);
			}
		);


		// elegant close io_context
		signals_ = std::make_unique<boost::asio::signal_set>(pool_->getNext(), SIGINT, SIGTERM);
		signals_->async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop_.exchange(true)) {
				return;
			}

			LOG_INFO("received signal, stopping client");
			}
		);

		if (!stop_) {
			LOG_INFO("ClientService running....");

			g_packetParser->Init();
		}
	}
	catch (std::exception const& e) {
		LOG_CRITICAL("Exception: {}", e.what());
		return false;
	}

	return !stop_;
}

void ClientService::run()
{
	// main thread handle
	std::string input;	
	while (!stop_.load(std::memory_order_acquire))
	{
		// send message in main_thread
		if (!(std::cin >> input)) {
			stop_.store(true, std::memory_order_release);
			break;
		}
		if (stop_ || !connector_->isConnected()) {
			break;
		}		
		connector_->send(encode_packet((uint32_t)MsgId::ECHO_REQ, input.c_str(), input.size()));
		input.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
	}
}

void ClientService::stop() {

	stop_.store(true, std::memory_order_release);
	connector_->Stop();
	pool_->stop();
}