#pragma once

#include <string>
#include <memory>

#include <google/protobuf/message_lite.h>

#include "networkEx/session.h"
#include "networkEx/connector.h"

class Player: std::enable_shared_from_this<Player> {
public:
	Player() = default;
	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }
	{
	}

	void send(uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
	void send(uint32_t msgId, const char* data ,uint16_t len);
	void replyPing();
	//void recv(uint32_t msgId, const char* data, uint16_t len);

	inline void setConnector(SharedConnectorPtr conn) {
		connector_ = conn;
	}
	inline uint64_t id()const { return id_; }
	inline std::string const& name()const { return name_; }
	inline void changeName(std::string const& newName) { name_ = newName; }
private:
	inline void send(std::string const& msg) {
		if (connector_) {
			connector_->send(msg);
		}
	}

public:
	void sendLoginReq();
	void sendAttack(uint32_t skillId);
	void sendMove(uint16_t x, uint16_t y);

	// Fill player data
	void fillData();

public:
	uint64_t getPlayerID()const { return id_; }
	std::string getPlayerName()const { return name_; }
private:
	uint64_t id_;
	std::string name_;
private:
	SharedConnectorPtr connector_{};
};

using PlayerPtr = std::shared_ptr<Player>;
