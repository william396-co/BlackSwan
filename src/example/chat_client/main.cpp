#include <boost/asio.hpp>
#include <iostream>


using boost::asio::ip::tcp;

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

		// elegant close io_context
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&](auto, auto) { io_context.stop(); });

		tcp::resolver resolver(io_context);
		tcp::resolver::results_type endpoints = resolver.resolve(host, port);


		bool connected = false;
		tcp::socket socket(io_context);
		boost::asio::async_connect(socket, endpoints,
			[&connected](boost::system::error_code const& error, tcp::endpoint)
			{
				if (!error) {
					connected = true;
				}
				else {
					std::cerr << "connected failed\n";
				}
			});



		// read and write work thread
		std::jthread read_work(
			[&socket, &io_context, &connected]() {

				// input the message 
				std::string str;
				std::array<char, 128> buf;
				while (connected && !io_context.stopped()) {
					std::cin >> str;

					socket.async_send(boost::asio::buffer(str, str.size()),
						[](boost::system::error_code const& error, size_t len) {
							if (!error) {
								std::cout << "finished send size: " << len << "\n";
							}
						});
					buf = {};
					socket.async_read_some(boost::asio::buffer(buf, sizeof(buf)), [&buf](boost::system::error_code const& error, size_t) {
						if (error) {
							std::cerr << error.what() << "\n";
						}
						else {
							std::cout << " recived from server:[" << buf.data() << "]\n";
						}
						});
				}
			});

	

		while (!io_context.stopped()) 
		{
			std::cout << "main loop\n";
			io_context.run();
		}
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    return 0;
}