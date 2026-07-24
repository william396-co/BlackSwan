#pragma once

#include <boost/asio.hpp>
#include <memory>

#include "socket.h"
#include "servicePool.h"

using boost::asio::ip::tcp;

class AsioTcpAcceptor : public std::enable_shared_from_this<AsioTcpAcceptor>
{
public:

    AsioTcpAcceptor(boost::asio::io_context& io,
        IoServicePoolPtr pool, tcp::endpoint const& ep)
        :pool_{ pool },
        acceptor_(std::make_shared<tcp::acceptor>(io, ep))
    {

    }
    ~AsioTcpAcceptor() {
        acceptor_->close();
    }

    void startAccept(std::function<void(SharedSocketPtr)> callback) {
        doAccept(callback);
    }
private:
    void doAccept(std::function<void(SharedSocketPtr)>callback) {

        auto& io = pool_->pickIoContext();

        auto socket = std::make_shared<SharedSocket>(tcp::socket(io), io);

        acceptor_->async_accept(
            socket->socket(),
            [self = shared_from_this(), callback, socket, this](boost::system::error_code const& error) {
                if (!error) {       
                    callback(socket);
                    /*
                    boost::asio::post(socket->io_context(), 
                        [=]() {
                            callback(socket);
                        });*/
                }
                else {
                    std::cerr << error.what() << "\n";
                }
                doAccept(callback);
            }
        );
    }
public:
    AsioTcpAcceptor(AsioTcpAcceptor const&) = delete;
    AsioTcpAcceptor& operator=(AsioTcpAcceptor const&) = delete;

    AsioTcpAcceptor(AsioTcpAcceptor&&) = delete;
    AsioTcpAcceptor& operator=(AsioTcpAcceptor&&) = delete;
private:
    IoServicePoolPtr pool_;
    std::shared_ptr<tcp::acceptor> acceptor_;
};
using AsioTcpAcceptorPtr = std::shared_ptr<AsioTcpAcceptor>;