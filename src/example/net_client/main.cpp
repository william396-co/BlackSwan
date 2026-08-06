#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>

#include <boost/asio.hpp>

#include "network/socket.h"
#include "network/session.h"
#include "network/connector.h"
#include "network/servicePool.h"


using boost::asio::ip::tcp;

enum ConIdx {
    idx_first=1,
};

int main(){


    std::unordered_map<ConIdx, AsioTcpSessionPtr> sessionList;
        
    auto host = "127.0.0.1";
    auto port = 9527;

    IoServicePoolPtr pool = std::make_shared<IoServicePool>(2, 1);
    pool->start(1);

    auto ep = tcp::endpoint(boost::asio::ip::make_address(host), port);
    AsioTcpConnector connector(pool);

    connector.asyncConnect(ep,std::chrono::nanoseconds{1000},
        [&sessionList](SharedSocketPtr socketPtr) {
            std::cout << "connect success:" << socketPtr->socket().local_endpoint()
                << "-->" << socketPtr->socket().remote_endpoint() << "\n";

            auto session = std::make_shared<AsioTcpSession>(socketPtr, 1024 * 8, 
                [](const char* data, size_t len)->size_t {
                    std::cout << "recv data " << data << "\n";
                    return len;
                });
            sessionList.emplace(ConIdx::idx_first, session);
        },
        []() {
            std::cerr << "connect failed\n";
        });


    // mainLoop
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds{ 1 });

        // on timer send message 
        for (auto const& s : sessionList) {
            s.second->send("hello this");
        }
    }

    return 0;
}