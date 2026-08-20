#include <iostream>
#include <memory>
#include <atomic>
#include <deque>

#include "networkEx/server.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"

#include "player.h"
#include "packetParser.h"

using MessageList = std::deque<std::pair<SessionPtr, std::string>>;
MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view, SessionPtr const& sender) {
	std::lock_guard lock(message_mtx);
	message_list.emplace_back(sender, std::string(data_view));
}

constexpr auto listen_port = 8321;
int main()
{
	std::cout << "GateServer starting....\n";

	std::atomic_bool stop = false;
	constexpr auto port = static_cast<boost::asio::ip::port_type>(listen_port);

	try {

		// io running pool 
		auto pool = std::make_shared<IoContextPool>(
			IoContextPool::DefaultPoolSize(),
			IoContextPool::DefaultConcurrencyHint());
		pool->run();

		// server
		auto server = std::make_unique<Server>(pool, port);
		server->start(
			[](auto session) {// accept Handle
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Session fd:" << s->fd() << " Send PING\n";
						s->send(encode_net_packet(CMD_PING, "PING", sizeof("PING"), 0));
					});
				//room.join(session);// gateSession
				(void)session;
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				return g_packetParser->onRecvData(data, len, session);
			},
			[](auto session) {// Disconnected Handle
				//room.leave(session);// gateSession
				(void)session;
			}
		);

		boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
		signals.async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}

			std::cout << "\n[system] received signal, stopping server\n";
			}
		);

		// ×¢²áÐ­Òé
		g_packetParser->Init();


		if (!stop) {
			std::cout << "GameServer running, listen port:[" << listen_port << "]\n";
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
		//room.stop();

		// Logic Level

		// IO Level
		server->stop();
		pool->stop();
	}
	catch (std::exception const&e) {
		std::cerr << "Exception:" << e.what() << "\n";
	}

	
	return 0;
}