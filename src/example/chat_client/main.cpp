
#include <iostream>
#include <string_view>

#include "networkEx/packet.h"
#include "networkEx/session.h"
#include "networkEx/connector.h"
#include "networkEx/ioContextPool.h"
#include "proto/protocol.h"

using MessageList = std::deque<std::pair<SessionPtr, std::string>>;

MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view,SessionPtr session) {
	//std::lock_guard lk(message_mtx);
	//message_list.push_back({ sender,std::string(data_view) });
	(void)session;
	std::cout << "received data:[" << data_view << "]\n";
}


int main(int argc,char** argv){

    std::cout<<"Chat Client\n";

    auto host = "127.0.0.1";
    uint16_t port = 9527;

	try {

		if (argc >= 3) {
			std::cerr << "Usage: client <host> <port>" << std::endl;
			host = argv[1];
			port = (uint16_t)strtoul(argv[2],nullptr,10);
		}


		std::atomic_bool  stop = false;

		// io running pool
		auto pool = std::make_shared<IoContextPool>(1, 1);
		pool->start();

		// connector 
		auto connector = std::make_shared<Connector>(pool->getNext());
		connector->SetDisconnectProc([&stop](SessionPtr) {
			stop.store(true, std::memory_order_release);
			std::cout << "[system] disconnected from server\n";
			}
		);

		connector->asyncConnect(host, port,
			[](SessionPtr session) {
				std::cout << "connect successed:" << session->remote_ep() << "\n";
				session->SetDataProc([](const char* data, size_t len, SessionPtr session)->size_t {// decode call back
					const char* recv_buf = data;
					Packet pack;
					while (len) {
						if (!decode_packet(recv_buf, len, pack)) {
							break;
						}
						len -= pack.size();
						recv_buf += pack.size();
						handleMessage(std::string_view(pack.data, pack.sz), session);
					}
					return len;
					});
			},
			[&stop](tcp::endpoint ep) {
				stop.store(true, std::memory_order_release);
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
			connector->send(encode_packet((uint32_t)MsgId::CHAT_REQ, input.c_str(), input.size()));
			input.clear();
			std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
		}

		connector->Stop();
		pool->stop();

    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    return 0;
}
