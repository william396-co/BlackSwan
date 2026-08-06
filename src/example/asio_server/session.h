#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <string>


using SessionCallback = std::function<void(std::string const&, std::string const&)>;

constexpr auto REPLY_SIZE = 32;

using  boost::asio::ip::tcp;
class Session: public std::enable_shared_from_this<Session>
{
    
public:
    Session(boost::asio::io_context&io);
    void start();

    inline tcp::socket &socket() { return socket_; }
	inline void setCallback(SessionCallback callback) { callback_ = callback; }

private:
    void init_handler(boost::system::error_code const& error);
    void analysis_handler(boost::system::error_code const& error);
    void done_handler(boost::system::error_code const& error);

    void after_read_handler(boost::system::error_code const& error, size_t readsize);
    void after_write_handler(boost::system::error_code const& error);
private:
    tcp::socket socket_;
    SessionCallback callback_{};
    char msg_[1024];
    std::string currMsg_;
    int sumSize_;
    unsigned int maxSize_;
};