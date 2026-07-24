#pragma once


#include <vector>
#include <thread>
#include <mutex>
#include <memory>

#include <boost/asio.hpp>

#include "wrapperService.h"

class IoServiceThread
{
public:
    IoServiceThread(int concurrencyHint)
        :ioService_{ concurrencyHint }
    {

    }
    ~IoServiceThread() {
        stop();
    }


    void stop() {
        std::lock_guard lk(mtx_);
        ioService_.stop();
        for (auto& t : threads_) {
            try {
                t.join();
            }
            catch (...) {
                
            }
        }
        threads_.clear();
    }
    void start(size_t threadNum) {
        std::lock_guard lk(mtx_);
        if (threads_.size() > 0)return;
        for (size_t i = 0; i < threadNum;++i) {
            threads_.emplace_back([this]() {
                ioService_.run();
            });
        }
    }

    boost::asio::io_context& io_context() {
        return ioService_.io_context();
    }
    WrapperIoService& wrapperIoService() { return ioService_; }

public:
    IoServiceThread(IoServiceThread const&)=delete;
    IoServiceThread& operator=(IoServiceThread const&)=delete;

    IoServiceThread(IoServiceThread&&)=delete;
    IoServiceThread& operator=(IoServiceThread&&)=delete;
private:
    std::mutex mtx_;
    std::vector<std::thread> threads_;
    WrapperIoService ioService_;
};

using IoServiceThreadPtr = std::shared_ptr<IoServiceThread>;