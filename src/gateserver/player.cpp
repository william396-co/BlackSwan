#include "player.h"

#include "networkEx/packet.h"
#include "playerMgr.h"
#include "playerSessionMgr.h"

void Player::forward2Client(uint32_t msgId, const char* data, uint16_t len)
{
	if (auto s = player_session_) {
		LOG_DEBUG("msgID:{} len:{}", msgId, len);
		auto msg = encode_packet(msgId, data, len);
		s->forward2Client(std::move(msg));
	}
}

void Player::forward2Login(uint32_t msgId, const char* data, uint32_t len, TransID transID)
{	
	if (auto s = player_session_) {
		LOG_DEBUG("transID:{} msgID:{} len:{}", transID, msgId, len);
		auto msg = encode_inner_packet(msgId, data, len, transID);// encode by user
		s->forward2Login(std::move(msg));
	}
}

void Player::forward2Server(uint32_t msgId, const char* data, uint32_t len, TransID transID)
{
	if (getCurStateType() != FsmStateType::EFST_InGame) {
		return;
	}
	if (auto s = player_session_) {
		LOG_DEBUG("transID:{} msgID:{} len:{}", transID, msgId, len);
		auto msg = encode_inner_packet(msgId, data, len, transID);// encode by user
		s->forward2Server(std::move(msg));
	}
}


void Player::onUpdate() 
{
	// on timer check

}

void Player::onDestroy() 
{
	g_playerSessionMgr->delSession(player_session_->fd());
	g_playerMgr->releasePlayer(shared_from_this());
}