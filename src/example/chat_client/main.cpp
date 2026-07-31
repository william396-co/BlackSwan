
#include <iostream>
#include <string_view>

#include "networkEx/packet.h"
#include "networkEx/session.h"
#include "networkEx/connector.h"
#include "networkEx/ioContextPool.h"

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


#if 1
		std::atomic_bool  stop = false;

		auto pool = std::make_shared<IoContextPool>(1, 1);
		pool->run();

		auto connector = std::make_shared<Connector>(pool->getNext());
		connector->asyncConnect(host, port, std::chrono::seconds{5},
			[](SessionPtr session) {
				std::cout << "connect successed:" << session->remote_ep() << "\n";
				session->SetDataProc([](const char* data, size_t len,SessionPtr session)->size_t {// decode call back
					const char* recv_buf = data;
					while (len) {
						DecodePacket pack{};
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
			[](tcp::endpoint ep) {
				std::cerr << "connect [" << ep << "] failed\n";
			});



		// elegant close io_context
		boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
		signals.async_wait([&stop](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}
			std::cout << "\n[system] received signal, stopping client....\n";
			});

		// main thread
		std::string input;
		while (!stop)
		{
			// send message in main_thread
			std::cin >> input;
			connector->send(input);
			//std::cout << "main loop\n";
			std::this_thread::sleep_for(std::chrono::milliseconds{ 2 });
		}

		connector->Stop();
		pool->stop();
#else

		boost::asio::io_context io_context;
		std::atomic_bool  stop = false;

		std::shared_ptr<Session> session = std::make_shared<Session>(io_context);
		session->Connect(host, port);
		session->SetDataProc([](const char* data, size_t len)->size_t {

			const char* recv_buf = data;
			while (len) {
				DecodePacket pack{};
				if (!decode_packet(recv_buf, len, pack)) {
					break;
				}
				len -= pack.size();
				recv_buf += pack.size();
				handleMessage(std::string_view(pack.data, pack.sz));
			}
			return len;
			});


		// elegant close io_context
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&stop](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}
			std::cout << "\n[system] received signal, stopping client....\n";
			});
				

		// io_thread 
		std::jthread io_work([&io_context]() {
			io_context.run();
			});

		// main thread
		std::string input;
		while (!stop) 
		{
			// send message in main_thread
			std::cin >> input;
			session->send(input);
			//std::cout << "main loop\n";
			std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
		}

		boost::asio::post(io_context, [&session]() {
			session->stop();
		});

		if (io_work.joinable()) {
			io_work.join();
		}

#endif

    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    return 0;
}
