#pragma once

#include <memory>
#include <string>

#include "share/networkEx/session.h"
#include <google/protobuf/message_lite.h>

class Player;
using PlayerPtr = Player*;

class Connector;
using ConnectorPtr = Connector*;

class PlayerSession: public std::enable_shared_from_this<PlayerSession>
{
public:
	PlayerSession(SessionPtr s, ConnectorPtr game_conn, ConnectorPtr login_conn);
	PlayerSession(SessionPtr s, PlayerPtr player);
	~PlayerSession();

	void setPlayer(PlayerPtr p) { player_ = p; }
	PlayerPtr getPlayer()const { return player_; }

	void forward2Client(uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
	void forward2Client(std::string msg);
	void forward2Server(uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
	void forward2Server(std::string msg);
	void forward2Login(uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
	void forward2Login(std::string msg);

public:
	inline uint32_t fd()const { return client_session_ ? client_session_->fd() : 0; }
	TransID transID()const;

	inline std::string getClientIp()const {
		return client_session_ ? client_session_->remote_ep().address().to_string() : "";
	}
public:
	void Close();
private:
	SessionPtr client_session_{};
	ConnectorPtr game_conn_{};
	ConnectorPtr login_conn_{};
	PlayerPtr player_{};
};

using PlayerSessionPtr = std::shared_ptr<PlayerSession>;