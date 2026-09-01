#include "clientSession.h"

#include "share/networkEx/session.h"
#include "share/networkEx/connector.h"

ClientSession::ClientSession(SessionPtr s, Connector* game_conn, Connector* login_conn)
	:session_{ s }, game_conn_{ game_conn }, login_conn_{login_conn}
{
	std::cout << __FUNCTION__ << " fd " << session_->fd() << "\n";
}

ClientSession::ClientSession(SessionPtr s, Player* player)
	: session_{ s }, player_{ player }
{
	std::cout << __FUNCTION__ << " fd " << session_->fd() << "\n";
}

ClientSession::~ClientSession()
{
	player_ = {};
	game_conn_ = {};
	std::cout << __FUNCTION__ << " fd " << session_->fd() << "\n";
}

void ClientSession::forward2Client(std::string msg)
{
	if (auto s = session_) {
		s->send(std::move(msg));
	}
}

void ClientSession::forward2Server(std::string msg)
{
	if (game_conn_) {
		game_conn_->send(std::move(msg));
	}
}

void ClientSession::forward2Login(std::string msg)
{
	if (login_conn_) {
		login_conn_->send(std::move(msg));
	}
}
