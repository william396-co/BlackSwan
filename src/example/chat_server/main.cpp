#include <iostream>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>

#include "networkEx/session.h"
#include "networkEx/packet.h"
#include "networkEx/ioContextPool.h"
#include "networkEx/server.h"

using boost::asio::awaitable;
using boost::asio::ip::tcp;
using boost::asio::use_awaitable;

class Chater {
public:
	explicit Chater(std::string name)
		:name_{ std::move(name) }
	{
	}
	Chater() :Chater("unknown")
	{
	}
	void setName(std::string name) { name_ = std::move(name); }
	std::string const& getName()const { return name_; }
private:
	std::string name_;
	SessionPtr session_;
};

class ChatRoom {
	using ChatList = std::unordered_map<uint32_t, Chater>;

public:
	void join(SessionPtr session) {
		{
			std::lock_guard lock(session_mtx);
			session_map_.emplace(session->fd(), session);
		}
		std::cout << "[system] new user joined, current online number: " << session_map_.size() << "\n";
	}

	void leave(SessionPtr const& session) {
		{
			std::lock_guard lock(session_mtx);
			session_map_.erase(session->fd());
		}
		std::cout << "[system] user left, current online number: " << session_map_.size() << "\n";
	}

	void broadcast(std::string const& msg, SessionPtr const& sender) {
		std::cout << "broadcast msg:[" << msg << "]\n";
		SessionMap curr_sessions;
		{
			std::lock_guard lock(session_mtx);
			curr_sessions = session_map_;
		}

		for (auto& it : curr_sessions) {
			(void)sender;
			it.second->send(msg);
		}
	}

	void stop() {
		SessionMap curr_sessions;
		{
			std::lock_guard lock(session_mtx);
			curr_sessions.swap(session_map_);
		}

		for (auto& it : curr_sessions) {
			it.second->stop();
		}
		curr_sessions.clear();
	}

private:
	std::mutex session_mtx;
	SessionMap session_map_;
};

using MessageList = std::deque<std::pair<SessionPtr, std::string>>;

MessageList message_list;
std::mutex message_mtx;

void handleMessage(std::string_view data_view, SessionPtr const& sender) {
	std::lock_guard lock(message_mtx);
	message_list.emplace_back(sender, std::string(data_view));
}

int main() {
	try {
		ChatRoom room;
		std::atomic_bool stop = false;
		constexpr auto port = static_cast<boost::asio::ip::port_type>(9527);

		std::cout << "==================ChatRoom running==================\n";
		std::cout << "=====Listen port: 9527============\n";


		// io running pool 
		auto pool = std::make_shared<IoContextPool>(
			IoContextPool::DefaultPoolSize(),
			IoContextPool::DefaultConcurrencyHint());
		pool->run();

		// server
		auto server = std::make_unique<Server>(pool, port);
		server->start(
			[&room](auto session) {// accept Handle
				room.join(session);
			},
			[](const char* data, size_t len, auto session)->size_t {// Data Process
				const char* recv_buf = data;
				size_t remaining = len;

				while (remaining > 0) {
					DecodePacket packet{};
					if (!decode_packet(recv_buf, remaining, packet)) {
						break;
					}

					auto const packet_size = packet.size();
					remaining -= packet_size;
					recv_buf += packet_size;
					handleMessage(std::string_view(packet.data, packet.sz), session);
				}

				return remaining;
			},
			[&room](auto session) {// Disconnected Handle
				room.leave(session);
			});

		boost::asio::signal_set signals(pool->getNext(), SIGINT, SIGTERM);
		signals.async_wait([&](boost::system::error_code const& error, int) {
			if (error || stop.exchange(true)) {
				return;
			}

			std::cout << "\n[system] received signal, stopping server\n";
			});

		while (!stop.load()) {
			MessageList current_messages;
			{
				std::lock_guard lock(message_mtx);
				current_messages.swap(message_list);
			}
			// all message handle handling here(synchronizing)
			for (auto const& message : current_messages) {
				room.broadcast(message.second, message.first);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		room.stop();

		server->stop();
		pool->stop();
	}
	catch (std::exception const& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
