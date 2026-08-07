#pragma once

#include <string>
#include <memory>

#include "networkEx/session.h"
#include "networkEx/connector.h"

class Player {
public:
	Player() = default;
	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }
	{
	}

	inline void send(std::string const& msg) {
		if (session_) {
			session_->send(msg);
		}
	}
	inline void setSession(SessionPtr session) {
		session_ = session;
	}
	inline uint64_t id()const { return id_; }
	inline std::string const& name()const { return name_; }
	inline void changeName(std::string const& newName) { name_ = newName; }
private:
	uint64_t id_;
	std::string name_;
private:
	SessionPtr session_{};
};

using PlayerPtr = std::shared_ptr<Player>;