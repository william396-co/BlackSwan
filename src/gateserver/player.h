#pragma once

#include <string>

#include "networkEx/session.h"
#include "networkEx/connector.h"

class Player {
public:
	Player() = default;
	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }
	{
	}

	explicit Player(SessionPtr client_session, Connector* game_connector)
		: id_{ client_session->fd() },
		client_session_{ client_session },
		game_connector_{game_connector}
	{
	}

	void forward2Server(uint32_t msgId, const char* data, uint16_t len, uint32_t fd);
	void forward2Client(uint32_t msgId, const char* data, uint16_t len);

	inline void SetSession(SessionPtr session) {
		client_session_ = session;
	}
	inline uint64_t id()const { return id_; }
	inline std::string const& name()const { return name_; }
	inline void changeName(std::string const& newName) { name_ = newName; }
private:
	inline void send(std::string const& msg) {
		if (auto session = client_session_) {
			session->send(msg);
		}
	}
	void send(uint32_t msgId, const char* data, uint16_t len);
private:
	uint64_t id_;
	std::string name_;
private:
	SessionPtr client_session_{};
	Connector*  game_connector_{};
};

using PlayerPtr = std::shared_ptr<Player>;