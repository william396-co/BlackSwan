#pragma once

#include <string>
#include <memory>

#include "networkEx/session.h"
#include "networkEx/connector.h"

class Player {
public:
	Player() = default;
	explicit Player(uint32_t client_id, SessionPtr gateSession)
		:id_{ client_id }, client_fd_{ client_id }, gateSession_{ gateSession }
	{
		std::cout << __FUNCTION__ << "client_fd:" << client_fd_ << " gate_fd:" << gateSession_->fd() << "\n";
	}
	~Player() {
		std::cout << __FUNCTION__ << "client_fd:" << client_fd_ << " gate_fd:" << gateSession_->fd() << "\n";
	}
		
	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }
	{
	}

	void send(uint32_t msgId, const char* data, uint16_t len);
	void recv(uint32_t msgId, const char* data, uint16_t len);

	inline void setSession(SessionPtr session) {
		gateSession_ = session;
	}
	inline uint32_t client_fd()const { return client_fd_; }
	inline uint64_t id()const { return id_; }
	inline std::string const& name()const { return name_; }
	inline void changeName(std::string const& newName) { name_ = newName; }
private:
	inline void send(std::string const& msg) {
		if (auto session = gateSession_) {
			session->send(msg);
		}
	}
private:
	uint64_t id_{};
	uint32_t client_fd_{};
	std::string name_;	
private:
	SessionPtr gateSession_{};
};

using PlayerPtr = std::shared_ptr<Player>;