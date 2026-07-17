#pragma once

#include <chrono>
#include <boost/asio.hpp>
#include <iostream>


class PeriodicTimer {
public:
	PeriodicTimer(boost::asio::io_context* io, std::chrono::milliseconds interval, int max_count)
		:timer_(io),
		interval_(interval),
		max_count_{ max_count }
	{
	}

	void Start() {

	}

	void onTimer(boost::system::error_code error) {
		if (error)return;


		++count_;
		std::cout << "execute " << count_ << " times\n";
		if (count_ < max_count_) {

		}
	}
private:
	boost::asio::steady_timer timer_;
	std::chrono::milliseconds interval_{};
	int max_count_{};
	int count_{};
};


void period_timer_test();