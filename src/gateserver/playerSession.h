#pragma once

#include <memory>
#include <string>

#include "share/networkEx/session.h"

class Player;
using PlayerPtr = Player*;

class Connector;
using ConnectorPtr = Connector*;

class PlayerSession {
public:
	PlayerSession(SessionPtr s, ConnectorPtr game_conn, ConnectorPtr login_conn);
	PlayerSession(SessionPtr s, PlayerPtr player);
	~PlayerSession();

	void setPlayer(PlayerPtr p) { player_ = p; }
	PlayerPtr getPlayer()const { return player_; }

	void forward2Client(std::string msg);
	void forward2Server(std::string msg);
	void forward2Login(std::string msg);

	inline uint32_t fd()const { return client_session_ ? client_session_->fd() : 0; }
private:
	SessionPtr client_session_{};
	ConnectorPtr game_conn_{};
	ConnectorPtr login_conn_{};
	PlayerPtr player_{};
};

using PlayerSessionPtr = std::shared_ptr<PlayerSession>;