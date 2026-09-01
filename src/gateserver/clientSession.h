#pragma once

#include <memory>

#include "share/networkEx/connector.h"

class Player;
class Connector;
class ClientSession {
public:
	ClientSession(SessionPtr s, Connector* game_conn,Connector* login_conn);
	ClientSession(SessionPtr s, Player* player);
	~ClientSession();

	void setPlayer(Player* p) { player_ = p; }
	Player* getPlayer()const { return player_; }

	void forward2Client(std::string msg);
	void forward2Server(std::string msg);
	void forward2Login(std::string msg);

	inline uint32_t fd()const { return session_ ? session_->fd() : 0; }
private:
	SessionPtr session_{};
	Connector* game_conn_{};
	Connector* login_conn_{};
	Player* player_{};
};

using ClientSessionPtr = std::shared_ptr<ClientSession>;