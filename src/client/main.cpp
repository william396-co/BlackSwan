#include <iostream>
#include <memory>
#include <atomic>


#include "networkEx/connector.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"

#include "player.h"
#include "packetParser.h"



using MessageList = std::deque<std::pair<SessionPtr, std::string>>;

MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view, SessionPtr session) {
	//std::lock_guard lk(message_mtx);
	//message_list.push_back({ sender,std::string(data_view) });
	(void)session;
	std::cout << "received data:[" << data_view << "]\n";
}


int main(int argc,char** argv) {

	std::cout << "Client....\n";   
	auto host = "127.0.0.1";
	uint16_t port = 9527;

	std::atomic_bool  stop = false;


	if (argc > 1) {
		port = (uint16_t)strtoul(argv[1], nullptr, 10);
	}

	try {
		
		auto pPlayer = std::make_unique<Player>();


		// io running pool
		auto pool = std::make_shared<IoContextPool>(1, 1);
		pool->start();

		// connector 
		auto connector = std::make_shared<Connector>(pool->getNext());
		connector->SetDisconnectProc([&stop,&pPlayer](SessionPtr) {
			stop.store(true, std::memory_order_release);
			pPlayer->setConnector(nullptr);
			std::cout << "[system] disconnected from server\n";
			}
		);

		connector->asyncConnect(host, port,
			[&connector, &pPlayer](SessionPtr session) {
				std::cout << "connect successed:" << session->remote_ep() << "\n";		
				pPlayer->setConnector(connector);
				session->StartHeartbeat(
					[](SessionPtr s) {
						std::cout << "Player Send GateServer PING\n";
						s->sendPing();
					});
				session->SetDataProc([&pPlayer](const char* data, size_t len, SessionPtr session)->size_t {// decode call back
					(void)session;
					return g_packetParser->onRecvData(data, len, pPlayer.get());
					});
			},
			[](tcp::endpoint ep) {
				std::cerr << "connect [" << ep << "] failed\n";
			}
		);


		// elegant close io_context
		boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
		signals.async_wait([&stop](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}
			std::cout << "\n[system] received signal, stopping client....\n";
			}
		);

		// 注册协议
		g_packetParser->Init();

		// main thread handle
		std::string input;
		while (!stop.load(std::memory_order_acquire))
		{
			// send message in main_thread
			if (!(std::cin >> input)) {
				stop.store(true, std::memory_order_release);
				break;
			}
			if (stop || !connector->isConnected()) {
				break;
			}
			pPlayer->send((uint32_t)MsgId::ECHO_REQ, input.c_str(), input.size());
			input.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
		}

		connector->Stop();
		pool->stop();

	}
	catch (std::exception const& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
