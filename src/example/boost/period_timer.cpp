#include "period_timer.h"

#include <iostream>
#include <boost/asio.hpp>


#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>


using boost::asio::awaitable;
using boost::asio::use_awaitable;

// coroutine interval timer,like sync code readable and understandable
awaitable<void> periodicTask(int intervalMs, int count) {

	auto executor = co_await boost::asio::this_coro::executor;

	boost::asio::steady_timer timer(executor);

	for (int i = 0; i < count;++i) {
		timer.expires_after(std::chrono::milliseconds{ intervalMs });

		// co_await hange current coroutine, auto-ressume on time
		co_await timer.async_wait(use_awaitable);

		std::cout << "execute " << i + 1 << " times\n";
	}
	std::cout << "finished\n";
}

void period_timer_test()
{
	boost::asio::io_context io;

#if 0

	PeriodicTimer pt(io, std::chrono::milliseconds{ 500 }, 5);
	pt.Start();


	io.run();
	std::cout << "finished\n";
#else
	boost::asio::co_spawn(io, periodicTask(500, 5), boost::asio::detached);
	io.run();

#endif
}

void cancel_timer_test() 
{
	boost::asio::io_context io;
	boost::asio::steady_timer timer(io, std::chrono::seconds{ 10 });

	timer.async_wait([](boost::system::error_code const& error) {
		if (error == boost::asio::error::operation_aborted) {// check the handler be cancelled
			std::cout << "Timer be cancelled\n";
		}
		else {
			std::cout << "Timer normal on Time\n";
		}
	});

	// cacncel timer, all waiting handler will set operation_aborted
	std::size_t cancelled = timer.cancel();
	std::cout << cancelled << " waiting operation  was cancelled\n";

	io.run();
}

