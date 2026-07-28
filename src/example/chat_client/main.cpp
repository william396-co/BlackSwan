
#include <iostream>
#include <string_view>

#include "networkEx/packet.h"
#include "networkEx/chatSession.h"

using MessageList = std::deque<std::pair<ChatSessionPtr, std::string>>;

MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view) {
	//std::lock_guard lk(message_mtx);
	//message_list.push_back({ sender,std::string(data_view) });
	std::cout << "received data:[" << data_view << "]\n";
}


int main(int argc,char** argv){

    std::cout<<"Chat Client\n";

    auto host = "127.0.0.1";
    auto port = "9527";

	try {

		if (argc >= 3) {
			std::cerr << "Usage: client <host> <port>" << std::endl;
			host = argv[1];
			port = argv[2];
		}

		boost::asio::io_context io_context;

		std::shared_ptr<ChatSession> session = std::make_shared<ChatSession>(io_context);
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


		bool stop = false;
		// elegant close io_context
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&stop](auto, auto) {
			std::cout << "stoping client....\n";
			stop = true;
			});
				

		// io_thread 
		std::jthread io_work([&stop, &io_context]() {
			while (!stop) {
				io_context.run();
			}
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
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    return 0;
}