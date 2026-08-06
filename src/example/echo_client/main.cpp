#include <boost/asio.hpp>
#include <iostream>


using boost::asio::ip::tcp;

int main(int argc,char** argv){

    std::cout<<"Echo Client\n";

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

		tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);


		// read and write work thread
		std::jthread read_write_work(
			[&socket, &io_context]() {

				std::array<char, 128> buf;
				while (!io_context.stopped()) {
					// input the message 
					std::string str;
					std::cin >> str;

					socket.send(boost::asio::buffer(str, str.size()));

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


		io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    return 0;
}