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

#include "networkEx/packet.h"
#include "networkEx/chatSession.h"


// forward declaration
class ChatSession;

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


using MessageList = std::deque<std::pair<ChatSessionPtr, std::string>>;

MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view,ChatSessionPtr sender) {
	std::lock_guard lk(message_mtx);
	message_list.push_back({ sender,std::string(data_view) });
}

#if 0
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
		if (socket_.is_open()) {
			std::cout << "[" << socket_.remote_endpoint() << "]" << __PRETTY_FUNCTION__ << "\n";
		}
		stop();
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

				std::fill(std::begin(buf), std::end(buf), '\0');
				auto n = co_await socket_.async_read_some(boost::asio::buffer(buf), use_awaitable);

				// TODO async_read_some callback
				handleMessage(std::string_view(buf, n), shared_from_this());
				std::cout << "recived data from [" << socket_.remote_endpoint() << "]:" << buf << "\n";
				//room_.broadcast(std::string(buf, n), shared_from_this());
			}
		}
		catch (boost::system::error_code const&error) {
			// 在这里统一处理所有断开情况
			if (error == boost::asio::error::eof) {
				std::cout << "connection disconnected:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::connection_reset) {
				std::cout << "connection reset:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::broken_pipe) {
				std::cout << "connection broken pipe:" << socket_.remote_endpoint() << "\n";
			}
			else {
				std::cout << "other network error:" << error.message() << "\n";
			}
			// 不需要手动 close，socket 析构时会自动关闭
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
		catch (boost::system::error_code const& error) {
			// 在这里统一处理所有断开情况
			if (error == boost::asio::error::eof) {
				std::cout << "connection disconnected:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::connection_reset) {
				std::cout << "connection reset:" << socket_.remote_endpoint() << "\n";
			}
			else if (error == boost::asio::error::broken_pipe) {
				std::cout << "connection broken pipe:" << socket_.remote_endpoint() << "\n";
			}
			else {
				std::cout << "other network error:" << error.message() << "\n";
			}
			// 不需要手动 close，socket 析构时会自动关闭
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
#endif

void ChatRoom::broadcast(std::string const& msg, ChatSessionPtr sender)
{
	std::cout << "broadcast msg:[" << msg << "]\n";
	for (auto& s : sessions_) {
		if (s == sender)continue;
		s->send(msg);
	}
}

awaitable<void> listener(tcp::acceptor acceptor, ChatRoom& room) {

	for (;;) {
#if 1
		auto socket = co_await acceptor.async_accept(use_awaitable);
		std::cout << "[system] new connection:" << socket.remote_endpoint() << "\n";

		// create new session and active it
		auto chatSession = std::make_shared<ChatSession>(std::move(socket));
		room.join(chatSession);
		chatSession->Start();

		chatSession->SetDataProc([&chatSession](const char* data, size_t len)->size_t {			
			const char* recv_buf = data;
			while (len) {
				DecodePacket pack{};
				if (!decode_packet(recv_buf, len, pack)) {
					break;
				}
				len -= pack.size();
				recv_buf += pack.size();
				handleMessage(std::string_view(pack.data, pack.sz), chatSession);
			}
			return len;
		});

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

		bool stop = false;

		std::cout << "==================ChatRoom running==================\n";
		std::cout << "=====List port: 9527============\n";		

		boost::asio::co_spawn(io,
			listener(tcp::acceptor(io, tcp::endpoint(tcp::v4(), port)), room), boost::asio::detached);

		// elegant close server
		boost::asio::signal_set signals(io, SIGINT, SIGTERM);
		signals.async_wait([&stop](auto ,auto) {
			std::cout << "\n[system] recieved signal, stop the serveice\n";			
			stop = true;
			});


		// io_thread
		std::jthread io_work(
			[&io,&stop]() {
				while (!stop) {
					io.run();
				}
			}
		);

		// main thread
		while (!stop) {
			// swap message from io_thread to main_thread
			MessageList currList;
			{
				std::lock_guard lk(message_mtx);
				currList.swap(message_list);
			}
			// handle message in main_thread
			for (auto const& it : currList) {
				room.broadcast(it.second,it.first);
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "Exception:" << e.what() << "\n";
	}
	return 0;
}