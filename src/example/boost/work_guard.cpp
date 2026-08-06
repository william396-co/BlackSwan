#include "work_guard.h"

#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>

void work_guard_test()
{
    boost::asio::io_context io;

    // work_guard increment work_count plust one, make run() thought still have work to do
    auto guard = boost::asio::make_work_guard(io);

    // another thread wait for 5 second
    std::jthread timer([&guard]() {
        std::this_thread::sleep_for(std::chrono::seconds{5});

        std::cout << " finished wait for 5 second\n";
        guard.reset(); // work_count dec one, let run() exit
    });

    std::cout << "run() is blocked until the work guard is released\n";
    io.run();

    std::cout << "===== finish run()========\n";
    //timer.join();
}

