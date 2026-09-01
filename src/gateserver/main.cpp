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
#include "clientSession.h"
#include "clientSessionMgr.h"


int main(int argc, char** argv)
{
	std::cout<<"GateServer\n";

	// TODO accept Client 
	// TODO forward client message to GameServer
	// TODO forward Server message to Client


	std::atomic_bool stop = false;
	uint16_t gate_port = 9527;
	constexpr auto game_port = 8321;
	constexpr auto login_port = 8600;
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
		auto server_connector = std::make_unique<Connector>(pool->getNext());
		server_connector->SetDisconnectProc([](SessionPtr) {
			std::cerr << "[system] GameServer Connector disconnected\n";
			});

		// Async Connector to GameSever
		server_connector->asyncConnect(host, game_port,
			[](SessionPtr session) {
				std::cout << "Connect successed:" << session->remote_ep() << "\n";
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Session fd:" << s->fd() << " Send GameServer PING\n";
						s->sendPing(0);
					});
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {
					return g_packetParser->onRecvServerData(data, len, session);
					});
			},
			[](tcp::endpoint ep) {
				std::cerr << "[system] Connect GameServer " << ep << " failed\n";
			}
		);


		// loginServer connector to LoginServer
		auto login_connector = std::make_unique<Connector>(pool->getNext());
		login_connector->SetDisconnectProc([](SessionPtr) {
			std::cerr << "[system] LoginServer Connector disconnected\n";
			});

		// Async Connector to LoginServer
		login_connector->asyncConnect(host, login_port,
			[](SessionPtr session) {
				std::cout << "Connect successed:" << session->remote_ep() << "\n";
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Session fd:" << s->fd() << " Send LoginServer PING\n";
						s->sendPing(0);
					});
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {
					return g_packetParser->onRecvLoginData(data, len, session);
					});
			},
			[](tcp::endpoint ep) {
				std::cerr << "[system] Connect LoginServer " << ep << " failed\n";
			}
		);

		// GateServer Listen Client Connect
		auto server = std::make_unique<Server>(pool, gate_port);
		server->start(
			[game_conn = server_connector.get(),login_conn = login_connector.get()](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Session fd:" << s->fd() << " Send Client PING\n";
						s->sendPing();
					});
				g_clientSessionMgr->addSession(session, game_conn, login_conn);
				//g_playerCtrl->addPlayer(session, connector.get());// TODO notify player online	
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return g_packetParser->onRecvClientData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				//g_playerCtrl->delPlayer(session);// TODO notify player offline
				g_clientSessionMgr->delSession(session);
			}
		);

		// GameServer already start Service
		if (server_connector->isConnected() && login_connector->isConnected()) {
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
		server_connector->Stop();
		login_connector->Stop();
		pool->stop();

	}
	catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}
