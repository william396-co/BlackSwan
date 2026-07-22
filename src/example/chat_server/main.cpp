#include <iostream>

#include <unordered_set>
#include <memory>
#include <deque>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>


using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::use_awaitable;
using boost::asio::co_spawn;


// forward declaration
class ChatSession;
using ChatSessionPtr = std::shared_ptr<ChatSession>;

// ChatRoom, oragnzie alll the online user
class ChatRoom {
	using SessionList = std::unordered_set<ChatSessionPtr>;
public:
	void join(ChatSessionPtr session) {
		sessions_.emplace(session);
		std::cout << "[system] new User join, current online number:" << sessions_.size() << "\n";
	}
	void leave(ChatSessionPtr session) {
		sessions_.erase(session);
		std::cout << "[system] User Leave, current online number:" << sessions_.size() << "\n";
	}

	void broadcast(std::string const& msg, ChatSessionPtr sender);
private:
	SessionList sessions_;
};


class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
	ChatSession(tcp::socket socket, ChatRoom& room)
		: room_{ room }
		, socket_{ std::move(socket) }
		, writeTimer_{ socket_.get_executor() }
	{
		std::cout << "[" << socket_.remote_endpoint() << "]" << __PRETTY_FUNCTION__ << "\n";
	}
	~ChatSession() {

		std::cout << "[" << socket_.remote_endpoint() << "]" << __PRETTY_FUNCTION__ << "\n";
	}
	void Start() {
		room_.join(shared_from_this());

		// active write and read coroutine
		co_spawn(
			socket_.get_executor(),
			[self = shared_from_this()]() { return	self->readerLoop();	},
			boost::asio::detached);

		co_spawn(
			socket_.get_executor(),
			[self = shared_from_this()]() { return	self->writerLoop();	},
			boost::asio::detached);
	}
	void deliver(std::string const&msg) {
		writeQueue_.push_back(msg);
		// wake up write-coroutine by cancel timer once
		writeTimer_.cancel_one();
	}
private:
	awaitable<void> readerLoop() {

		try {

			//boost::asio::streambuf buf;
			char buf[1024*4];
			for (;;) {
#if 0
				auto n = co_await boost::asio::async_read_until(
					socket_, buf, '\n', use_awaitable);

				std::string line(boost::asio::buffers_begin(buf.data()),
					boost::asio::buffers_end(buf.data()) + n);
	
				buf.consume(n);
#else


				std::fill(std::begin(buf), std::end(buf), '\0');
				auto n = co_await socket_.async_read_some(boost::asio::buffer(buf), use_awaitable);
#endif			
				std::cout << "recived data from [" << socket_.remote_endpoint() << "]:" << buf << "\n";
				room_.broadcast(std::string(buf, n), shared_from_this());
			}
		}
		catch (std::exception&) {
			stop();
		}
	}
	awaitable<void> writerLoop() {

		try {

			while (socket_.is_open()) {
				
				// 设置一个"永不到期"的定时器作为通知机制
				// 当有新消息时，deliver() 会 cancel_one() 来唤醒这个等待
				if (writeQueue_.empty()) {

					writeTimer_.expires_at(boost::asio::steady_timer::time_point::max());
					
					// redirect_error：将异常转为 error_code
					// 这样 cancel 不会抛异常，而是返回 operation_aborted
					boost::system::error_code error;
					co_await writeTimer_.async_wait(boost::asio::redirect_error(use_awaitable, error));
					// ec == operation_aborted 表示被 deliver() 唤醒
				}

				// send the message in the queue
				while (!writeQueue_.empty()) 
				{
					co_await boost::asio::async_write(
						socket_,
						boost::asio::buffer(writeQueue_.front()), use_awaitable);
					writeQueue_.pop_front();
				}
			}

		}
		catch (std::exception&) {
			stop();
		}
	}
	void stop() {
		room_.leave(shared_from_this());
		boost::system::error_code error;
		socket_.close(error);
		writeTimer_.cancel();
	}
private:
	ChatRoom& room_;
	tcp::socket socket_;
	boost::asio::steady_timer writeTimer_;
	std::deque<std::string> writeQueue_;
};

void ChatRoom::broadcast(std::string const& msg, ChatSessionPtr sender)
{
	for (auto& s : sessions_) {
		if (s == sender)continue;
		s->deliver(msg);
	}
}

awaitable<void> listener(tcp::acceptor acceptor, ChatRoom& room) {

	for (;;) {
#if 1
		auto socket = co_await acceptor.async_accept(use_awaitable);
		std::cout << "[system] new connection:" << socket.remote_endpoint() << "\n";

		// create new session and active it
		std::make_shared<ChatSession>(std::move(socket), room)->Start();
#else
		std::make_shared<ChatSession>(co_await acceptor.async_accept(use_awaitable), room)->Start();
#endif
	}
}

int main() {

	try {

		boost::asio::io_context io;
		ChatRoom room;
		auto port = 9527;

		std::cout << "==================ChatRoom running==================\n";
		std::cout << "=====List port: 9527============\n";
		//std::cout << "使用 nc localhost 9527 或 telnet localhost 9527 连接\n";
		std::cout << "========================\n\n";

		boost::asio::co_spawn(io,
			listener(tcp::acceptor(io, tcp::endpoint(tcp::v4(), port)), room), boost::asio::detached);

		// elegant close server
		boost::asio::signal_set signals(io, SIGINT, SIGTERM);
		signals.async_wait([&io](auto ,auto) {
			std::cout << "\n[system] recieved signal, stop the serveice\n";
			io.stop();
			});

		io.run();
	}
	catch (std::exception const& e) {
		std::cerr << "Exception:" << e.what() << "\n";
	}
	return 0;
}