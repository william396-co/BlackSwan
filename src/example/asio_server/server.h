#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>

class Session;
using SessionPtr = std::shared_ptr<Session>;

using boost::asio::ip::tcp;
class Server
{
public:
    Server(boost::asio::io_context&io, tcp::endpoint const& ep);
    ~Server();

    void start();// start listen
    void run();// async
private:
    void callback_session(std::string const& fromIp, std::string const& info);
    void accept_handler(SessionPtr session, boost::system::error_code const& error);
private:
    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;
};