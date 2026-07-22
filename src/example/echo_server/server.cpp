#include "server.h"


// echo  function(original message send back)
awaitable<void> echo(tcp::socket socket) {


	try {
		char data[1024];
		for (;;) {

			std::fill(std::begin(data), std::end(data), '\0');
			size_t n = co_await socket.async_read_some(
				boost::asio::buffer(data), use_awaitable);

			std::cout << "[" << socket.remote_endpoint() << "] recieve data:[" << data << "]\n";

			co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
		}

	}
	catch (std::exception& e) {
		std::cout << "connection finished:" << e.what() << "\n";
	}
}

// acceptor listener function(accept a new socket)
awaitable<void> listener(tcp::acceptor acceptor) {

	for (;;) {
		
		// o_await：挂起，等待新连接
		tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
		std::cout << "new connection:" << socket.remote_endpoint() << "\n";
		
		// co_spawn：启动独立协程处理这个连接
		// detached：不关心返回值，协程独立运行
		boost::asio::co_spawn(
			acceptor.get_executor(),
			echo(std::move(socket)),
			boost::asio::detached);
	}
}