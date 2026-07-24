#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <memory>


#include "wrapperService.h"
#include "serviceThread.h"

class IoServicePool
{
public:
    IoServicePool(size_t poolSize, int concurrencyHint)
        :pickIoServiceIndex_{}
    {
        if (!poolSize)
            throw std::runtime_error("pool size is zero");

        for (size_t i = 0; i != poolSize;++i) {
            ioServicePool_.emplace_back(std::make_shared<IoServiceThread>(concurrencyHint));
        }
    }
    ~IoServicePool() {
        stop();
    }

    void start(size_t threadNumEveryService) {
        std::lock_guard lk(mtx_);
        for (auto& s : ioServicePool_) {
            s->start(threadNumEveryService);
        }
    }
    void stop() {
        std::lock_guard lk(mtx_);
        for (auto& s : ioServicePool_) {
            s->stop();
        }
        ioServicePool_.clear();
    }
    boost::asio::io_context& pickIoContext() {
        return pickIoServiceThread()->io_context();
    }
    std::shared_ptr<IoServiceThread> pickIoServiceThread() {

        auto index = pickIoServiceIndex_.fetch_add(1, std::memory_order_relaxed);
        return ioServicePool_[index % ioServicePool_.size()];
    }
public:    
    IoServicePool(IoServicePool const&)=delete;
    IoServicePool& operator=(IoServicePool const&)=delete;

    IoServicePool(IoServicePool&&)=delete;
    IoServicePool& operator=(IoServicePool&&)=delete;

private:

    std::vector<std::shared_ptr<IoServiceThread>> ioServicePool_;
    std::mutex mtx_;
    std::atomic_int32_t pickIoServiceIndex_{};
};

using IoServicePoolPtr = std::shared_ptr<IoServicePool>;