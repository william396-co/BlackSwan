#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>

class IoContextPool {
public:
	explicit IoContextPool(size_t poolSize)
		:nextIndex_{} {
		for (size_t i = 0; i < poolSize;++i) {
			// create indenpend io_context
			contexts_.emplace_back(std::make_unique<boost::asio::io_context>());

			guards_.emplace_back(boost::asio::make_work_guard(*contexts_.back()));
		}
	}

	// start all threads
	void run() {
		for (auto& ctx : contexts_) {
			threads_.emplace_back([&ctx]() {ctx->run();});
		}
	}
	void stop() {
		guards_.clear();
		for (auto& ctx : contexts_) {
			ctx->stop();
		}
		for (auto& t : threads_) {
			t.join();
		}
	}
	// round-robin choice next io_context	
	boost::asio::io_context& getNext() 
	{
		auto& ctx = *contexts_[nextIndex_];
		nextIndex_ = (nextIndex_ + 1) % contexts_.size();
		return ctx;
	}
private:
	std::vector<std::unique_ptr<boost::asio::io_context>> contexts_;
	std::vector<boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type>> guards_;
	std::vector<std::thread> threads_;
	size_t nextIndex_;
};