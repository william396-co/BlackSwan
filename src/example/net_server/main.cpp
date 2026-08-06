#include <iostream>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>

#include "network/socket.h"
#include "network/acceptor.h"
#include "network/servicePool.h"
#include "network/serviceThread.h"
#include "network/session.h"
#include "utils/xtime.h"

using boost::asio::ip::tcp;

void handleAccept(SharedSocketPtr socket) {
    std::cout << "new connect:" << socket->socket().remote_endpoint() << "\n";

    auto sessionPtr = std::make_shared<AsioTcpSession>(socket, 1024 * 6,
        [](const char* data, size_t len)->size_t {
            std::cout << "recived data:" << data << "\n";
            return len;
        });
    sessionPtr->startRecv();
}

int main(){


    bool stopped = false;
    auto pool = std::make_shared<IoServicePool>(4, 1);
    pool->start(1);

    IoServiceThread serveiceWrapper(1);
    serveiceWrapper.start(1);

	short port = 9527;

	auto acceptor = std::make_shared<AsioTcpAcceptor>(serveiceWrapper.io_context(),
		pool,
		tcp::endpoint(tcp::v4(), port));

	acceptor->startAccept([](SharedSocketPtr socket) {
		handleAccept(socket);
		});


    boost::asio::signal_set sig(serveiceWrapper.io_context(), SIGINT, SIGTERM);
    sig.async_wait([&](boost::system::error_code const& error, int signal) {
        stopped = true;
    });


    // mainLoop
    while (!stopped) {        
        std::this_thread::sleep_for(1s);
    }

    serveiceWrapper.stop();
    pool->stop();    

    return 0;
}