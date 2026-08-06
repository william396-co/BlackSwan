#include "post_vs_dispatch.h"


#include <boost/asio.hpp>
#include <iostream>
#include <thread>


void post_vs_dispatch_test() 
{
	boost::asio::io_context io;
	auto guard = boost::asio::make_work_guard(io);

	std::cout << "current threadId:" << std::this_thread::get_id() << "\n";

	std::jthread worker([&io] {io.run();});

	// === post：总是排入队列 ===
    // 无论当前线程是不是 io_context 线程，都排队等 run() 执行
	boost::asio::post(io, []() {
		std::cout << std::this_thread::get_id() << " [post] must run in the io_context thread\n";
	});

	// == = dispatch：看情况 == =
	// 如果当前线程正在 run() io_context → 立即执行
	// 否则 → 等同于 post
	boost::asio::dispatch(io, [] {
		std::cout <<std::this_thread::get_id()<< " [dispatch] may run immediatly, may in run queue\n";
		});

	std::cout << "current threadId: " << std::this_thread::get_id() << " running...\n";

	std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
	guard.reset();
}