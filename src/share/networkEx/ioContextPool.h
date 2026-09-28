#pragma once

#include <boost/asio.hpp>
#include <atomic>
#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <exception>
#include <stdexcept>
#include <functional>


using PostEvent = std::function<void()>;

class IoContextPool: public std::enable_shared_from_this<IoContextPool> {
public:
	static size_t DefaultPoolSize(uint32_t multi_core = 1) {// multi_core= 2 or 1,default is 1
		auto const count = std::thread::hardware_concurrency();
		return count == 0 ? 1u : count * multi_core;
	}

	static int DefaultConcurrencyHint() {
		return 1;
	}

	explicit IoContextPool(size_t poolSize, int concurrencyHint)
		:nextIndex_{} 
	{
		if (!poolSize)
			throw std::runtime_error("pool size is zero");
		for (size_t i = 0; i < poolSize;++i) {
			// create indenpend io_context
			contexts_.emplace_back(std::make_unique<boost::asio::io_context>(concurrencyHint));

			guards_.emplace_back(boost::asio::make_work_guard(*contexts_.back()));
		}
	}

	// start all threads
	void start() {
		for (auto& ctx : contexts_) {
			threads_.emplace_back([ctx = ctx.get()]() {
				//std::cout<<__func__ <<" threadId:"<<std::this_thread::get_id()<<"\n";
				ctx->run();
			});
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
		auto const index = nextIndex_.fetch_add(1, std::memory_order_relaxed);
		return *contexts_[index % contexts_.size()];
	}
private:
	std::vector<std::unique_ptr<boost::asio::io_context>> contexts_;
	std::vector<boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type>> guards_;
	std::vector<std::thread> threads_;
	std::atomic_size_t nextIndex_;
};

using IoContextPoolPtr = std::shared_ptr<IoContextPool>;
