#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>

#include "session.h"
#include "server.h"
#include "networkEx/ioContextPool.h"

using boost::asio::ip::tcp;

constexpr auto port = 9527;

std::mutex iomtx;

void sync_echo_server() {


    boost::asio::io_context io;


    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));
    std::cout << "sync Echo Server listening port:" << port << "\n";

    for (;;) {

        tcp::socket client_socket(io);
        acceptor.accept(client_socket);

        std::cout << "Client Connect: " << client_socket.remote_endpoint().address().to_string()
            << ":" << client_socket.remote_endpoint().port() << "\n";

        boost::system::error_code error;
        char buf[1024];
        while (true) {

            size_t n = client_socket.read_some(boost::asio::buffer(buf), error);
            if (error == boost::asio::error::eof) {
                std::cout << "Client disconnected\n";
                break;
            }
            if (error) {
                std::cerr << error.what() << "\n";
                //throw boost::system::system_error(error);
                break;
            }

            std::cout << "server received data:[" << buf << "]\n";

            // send received data back to client 
            boost::asio::write(client_socket, boost::asio::buffer(buf, n));
        }
    }
}


void async_echo_server() {

    boost::asio::io_context io;

    // elegant close io_context
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&io](auto, auto) { io.stop(); });
        
    Server server(io, port);    
    std::cout << "Async Echo Server Start listening by port: "<<port<<"\n";

    io.run();    
}




void coroutine_echo_server() {

    boost::asio::io_context io;

    // elegant close io_context
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&io](auto, auto) { io.stop(); });

    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));
    std::cout << "Croutine Echo Server Start listening by port:" << port << "\n";

    boost::asio::co_spawn(io, listener(std::move(acceptor)), boost::asio::detached);
    io.run();
}


void single_iocontex_multi_thread_test() {
    boost::asio::io_context io;
    //auto guard = boost::asio::make_work_guard(io);


    for (int i = 0; i != 10;++i) {
        boost::asio::post(io, [i]() {
            std::lock_guard lock(iomtx);
            std::cout << "task " << i << " running on thread:" << std::this_thread::get_id() << "\n";
            });
    }

    std::vector<std::jthread> threads;
    for (int i = 0; i != 4;++i) {
        threads.emplace_back([&io]() {io.run();});
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

   //  guard.reset();
}

void multi_iocontext_multithread_test() {

    IoContextPool pool(4);
    for (int i = 0; i != 10;++i) {
        boost::asio::post(pool.getNext(), [i]() {
            std::lock_guard lock(iomtx);
            std::cout << "task " << i << " running on thread:" << std::this_thread::get_id() << "\n";
            });
    }
    pool.run();

    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
    pool.stop();
}

void strand_test() {

    boost::asio::io_context io;
    auto strand = boost::asio::make_strand(io);

    auto counter = 0;

    srand(time(nullptr));
	

    // 4 个线程并发运行 io_context
    std::vector<std::thread> threads;
    for (int i = 0; i != 10;++i) {
        threads.emplace_back([&io] {
            //std::this_thread::sleep_for(std::chrono::milliseconds{ rand() % 10 });
            std::cout << "[" << std::this_thread::get_id() << "]io.run()\n";
            io.run();
            });
    }
    
    // 通过 strand 提交 1000 个递增任务
    for (int i = 0; i != 1000;++i) {
        // 通过 strand 提交 → 保证串行执行 → 无需加锁
        boost::asio::post(strand, [&counter] {
            counter++;
            std::cout << "[" << std::this_thread::get_id() << "]counter=" << counter << "\n";
            });
    }

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    // counter 一定是 1000（strand 保证了串行）
    std::cout << "counter = " << counter << "\n";
}

int main(){

#if 1
    async_echo_server();
#else
    sync_echo_server();
    single_iocontex_multi_thread_test();
    multi_iocontext_multithread_test();
    coroutine_echo_server();
    strand_test();
#endif
    
    return 0;
}