#pragma once

#include <chrono>
#include <boost/asio.hpp>
#include <iostream>


class PeriodicTimer {
public:
	PeriodicTimer(boost::asio::io_context& io, std::chrono::milliseconds interval, int max_count)
		:timer_{ io },
		interval_(interval),
		max_count_{ max_count }
	{
	}

	void Start() {

		// set time with current+interval
		timer_.expires_after(interval_);

		// async waiting, execute onTimer
		timer_.async_wait([this](boost::system::error_code const& error) {
			onTimer(error);
		});
	}
private:
	void onTimer(boost::system::error_code error) {

		if (error)return;

		++count_;
		std::cout << "execute " << count_ << " times\n";
		if (count_ < max_count_) {

			timer_.expires_at(timer_.expiry() + interval_);
			timer_.async_wait([this](boost::system::error_code const& error) {
				onTimer(error);
			});
		}
	}
private:
	boost::asio::steady_timer timer_;
	std::chrono::milliseconds interval_{};
	int max_count_{};
	int count_{};
};


void period_timer_test();
void cancel_timer_test();
