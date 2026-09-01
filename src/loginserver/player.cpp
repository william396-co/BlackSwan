#include "player.h"

#include "networkEx/packet.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "playerCtrl.h"

void Player::onDestroy()
{
	g_playerCtrl->releasePlayer(this);
	delete this;
}

void Player::onUpdate()
{
	if (getCurStateType() == FsmStateType::EFST_Login)
		return;
	// TODO
}

void Player::setPTID(std::string const& ptid_name)
{
	ptid_name_ = ptid_name;
	g_playerCtrl->addPtid2Map(ptid_name, id());
}

void Player::send(uint32_t msgId, const char* data, uint16_t len)
{
	//std::cout << __FUNCTION__ << " fd:" << client_session_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto gate_session = g_gateSessionMgr->getGateSession(gate_fd_);
	if(gate_session)
	{
		auto msg = encode_net_packet(msgId, data, len, gate_session->fd());
		gate_session->send(std::move(msg));
	}
}