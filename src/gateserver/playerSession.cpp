#include "playerSession.h"

#include "share/networkEx/connector.h"
#include "player.h"

PlayerSession::PlayerSession(SessionPtr s, ConnectorPtr game_conn, ConnectorPtr login_conn)
	:client_session_{ s }, game_conn_{ game_conn }, login_conn_{login_conn}
{
	LOG_DEBUG(" fd:{} ", fd());
}

PlayerSession::PlayerSession(SessionPtr s, PlayerPtr player)
	: client_session_{ s }, player_{ player }
{
	LOG_DEBUG(" fd:{} ", fd());
}

PlayerSession::~PlayerSession()
{
	LOG_DEBUG(" fd:{} ", fd());
	player_ = {};
	game_conn_ = {};
	login_conn_ = {};
}

void PlayerSession::forward2Client(std::string msg)
{
	if (auto s = client_session_) {
		s->send(std::move(msg));
	}
}

void PlayerSession::forward2Server(std::string msg)
{
	if (game_conn_) {
		game_conn_->send(std::move(msg));
	}
}

void PlayerSession::forward2Login(std::string msg)
{
	if (login_conn_) {
		login_conn_->send(std::move(msg));
	}
}
