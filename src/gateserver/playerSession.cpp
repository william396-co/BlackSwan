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

void PlayerSession::forward2Client(uint32_t msgId, ::google::protobuf::MessageLite& refMsg)
{
	auto bodySize = refMsg.ByteSizeLong();
	if (bodySize > std::numeric_limits<uint16_t>::max()) {
		LOG_CRITICAL("protoData Bodysize:{} over bufferSize:{} msgID:{}", bodySize, std::numeric_limits<uint16_t>::max(), msgId);
		bodySize = 0;
	}
	forward2Client(encode_packet(msgId, bodySize ? refMsg.SerializeAsString().c_str() : nullptr, (uint32_t)bodySize));
}

void PlayerSession::forward2Client(std::string msg)
{
	if (auto s = client_session_) {
		s->send(std::move(msg));
	}
}

void PlayerSession::forward2Server(uint32_t msgId, ::google::protobuf::MessageLite& refMsg)
{
	auto bodySize = refMsg.ByteSizeLong();
	if (bodySize > std::numeric_limits<uint32_t>::max()) {
		LOG_CRITICAL("protoData Bodysize:{} over bufferSize:{} msgID:{}", bodySize, std::numeric_limits<uint32_t>::max(), msgId);
		bodySize = 0;
	}
	forward2Server(encode_inner_packet(msgId, bodySize ? refMsg.SerializeAsString().c_str() : nullptr, (uint32_t)bodySize, transID()));
}

void PlayerSession::forward2Server(std::string msg)
{
	if (game_conn_) {
		game_conn_->send(std::move(msg));
	}
}

void PlayerSession::forward2Login(uint32_t msgId, ::google::protobuf::MessageLite& refMsg)
{
	auto bodySize = refMsg.ByteSizeLong();
	if (bodySize > std::numeric_limits<uint32_t>::max()) {
		LOG_CRITICAL("protoData Bodysize:{} over bufferSize:{} msgID:{}", bodySize, std::numeric_limits<uint32_t>::max(), msgId);
		bodySize = 0;
	}
	forward2Login(encode_inner_packet(msgId, bodySize ? refMsg.SerializeAsString().c_str() : nullptr, (uint32_t)bodySize, transID()));
}

void PlayerSession::forward2Login(std::string msg)
{
	if (login_conn_) {
		login_conn_->send(std::move(msg));
	}
}

TransID PlayerSession::transID() const
{
	return player_ ? player_->playerID() : 0;
}

void PlayerSession::Close()
{
	if (client_session_ && client_session_->isConnected()) {
		if (player_) {

			FsmEvent event{};
			event.isGlobalEvent = true;
			event.globalEvtType = EFsmGlobalEvtType::EGET_Client_Socket_Close;
			event.playerSession_ = shared_from_this();
			player_->onEvent(event);
		}
		client_session_->stop();
	}
}