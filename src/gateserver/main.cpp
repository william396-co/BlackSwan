#include <iostream>
#include <memory>
#include <deque>
#include <atomic>
#include <exception>
#include <stdexcept>
#include <thread>

#include "networkEx/connector.h"
#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "networkEx/session.h"
#include "proto/protocol.h"

#include "packetParser.h"
#include "player.h"
#include "playerCtrl.h"


int main(int argc, char** argv)
{
	std::cout<<"GateServer\n";

	// TODO accept Client 
	// TODO forward client message to GameServer
	// TODO forward Server message to Client


	std::atomic_bool stop = false;
	uint16_t gate_port = 9527;
	constexpr auto game_port = 8321;
	constexpr auto host = "127.0.0.1";

	if (argc > 1) {
		gate_port = (uint16_t)strtoul(argv[1], nullptr, 10);
	}

	try {

		std::cout << "GateServer starting....\n";

		auto pool = std::make_shared<IoContextPool>(IoContextPool::DefaultPoolSize(), IoContextPool::DefaultConcurrencyHint());
		pool->run();


		// Terminate Server SIGNAL
		boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
		signals.async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}

			std::cout << "\n[system] received signal, stopping server\n";
			}
		);


		// server connector to GameSever
		auto connector = std::make_unique<Connector>(pool->getNext());
		connector->SetDisconnectProc([&stop](SessionPtr) {
			stop.store(true, std::memory_order_release);
			std::cerr << "[system] GameServer Connector disconnected\n";
			});

		//TODO repeated connector until Connected success
		connector->asyncConnect(host, game_port, std::chrono::seconds{ 5 },
			[](SessionPtr session) {
				std::cout << "Connect successed:" << session->remote_ep() << "\n";
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {
					return g_packetParser->onRecvServerData(data, len, session);
					});
			},
			[&stop](tcp::endpoint ep) {
				stop.store(false, std::memory_order_release);
				std::cerr << "[system] Connect GameServer " << ep << " failed\n";
			}
		);

		// GateServer Listen Client Connect
		auto server = std::make_unique<Server>(pool, gate_port);
		server->start(
			[&connector](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Session fd:" << s->fd() << " Send PING\n";
						s->send(encode_packet(CMD_PING, "PING", sizeof("PING")));
					});
				g_playerCtrl->addPlayer(session, connector.get());// TODO notify player online
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return g_packetParser->onRecvClientData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				g_playerCtrl->delPlayer(session);// TODO notify player offline
			}
		);

		// GameServer already start Service
		if (connector->isConnected()) {
			std::cout << "GateServer running, listen port:[" << gate_port << "]\n";
		}

		// main thread handle
		while (!stop.load()) {
#if 0
			MessageList current_messages;
			{
				std::lock_guard lock(message_mtx);
				current_messages.swap(message_list);
			}
			// all message handle handling here(synchronizing)
			for (auto const& message : current_messages) {
				room.broadcast(message.second, message.first);
			}

#endif
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server->stop();
		connector->Stop();
		pool->stop();

	}
	catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}