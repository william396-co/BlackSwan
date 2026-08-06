#include "io_context.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>


void io_context_test()
{
    using namespace boost;
    using namespace boost::asio;


    io_context io;

    std::cout << "main thread Id:" << std::this_thread::get_id() << "\n";

    // post task to io_context
    asio::post(io, []() { 
        std::cout << "[task 1] threadId:" << std::this_thread::get_id() << "\n"; // same as main thread Id
        });

    asio::post(io, []() {
        std::cout << "[task 2] threadId:" << std::this_thread::get_id() << "\n";
        });

    asio::post(io, []() {
        std::cout << "[task 3] threadId:" << std::this_thread::get_id() << "\n";
        });

    // now the queue have 3 task
    // run will pop the task from the queue sequence, then execute 
    std::cout << "=====start run()====\n";
#if 0
    io.run();
#else
    io.run_one();// run task 1
    io.poll();// run task 2 and task 3
    io.poll_one();// no task, just return immediately
#endif

    std::cout << "=====finish run()====\n";
}
