#include <iostream>

#include <stdexcept>
#include <exception>


#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"
#include "log/log.h"

#include <google/protobuf/stubs/common.h>

#include "player.h"
#include "playerCtrl.h"
#include "packetParser.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "apMgr.h"

constexpr auto listen_port = 8600;
int main()
{		

		auto log = std::make_unique<CLog>("LoginServer");

		LOG_INFO("LoginServer starting....");

		std::atomic_bool stop = false;
		constexpr auto port = static_cast<boost::asio::ip::port_type>(listen_port);

		try {

			// io running pool 
			auto pool = std::make_shared<IoContextPool>(
				IoContextPool::DefaultPoolSize(),
				IoContextPool::DefaultConcurrencyHint());
			pool->run();

			// packetParser Init
			g_packetParser->Init();

			// server
			auto server = std::make_unique<Server>(pool, port);
			server->start(
				[](auto session) {// accept Handle
					session->StartHeartbeat(
						[](SessionPtr s) {
							LOG_INFO("Session fd {} Send LoginServer PING", s->fd());
							s->sendPing(0);
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

			boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
			signals.async_wait([&](boost::system::error_code const& error, int) {
				if (error || stop.exchange(true)) {
					return;
				}

				LOG_INFO("[system] received signal, stopping server");
				}
			);


			if (!stop) {
				LOG_INFO("LoginServer running, listen port:[{}]", listen_port);
			}

			// main thread handle
			while (!stop.load())
			{
				g_playerCtrl->onUpdate();
				g_packetParser->onUpdate();
				g_apmgr->onUpdate();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));// avoid busy loop
			}

			// IO Level
			server->stop();
			pool->stop();

		}
		catch (std::exception const& e) {
			std::cout << "Exception:" << e.what() << "\n";
		}


    return 0;
}
