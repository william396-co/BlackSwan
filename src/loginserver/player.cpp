#include "player.h"

#include "networkEx/packet.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "playerCtrl.h"
#include "proto/gg_ls.pb.h"
#include "proto/commdef.pb.h"
using namespace InnerCmd;
using namespace GG_LS_Cmd;
#include "proto/commdefs.h"

void Player::onDestroy()
{
	g_playerCtrl->releasePlayer(this);
	delete this;// be careful about this use
}

void Player::onUpdate()
{
	if (getCurStateType() == FsmStateType::EFST_Login)
		return;
	// TODO
}

void Player::setPTID(std::string const& szPTID)
{
	szPTID_ = szPTID;
	g_playerCtrl->addPtid2Map(szPTID_, id());
}

void Player::setLoginData(AuthInfoPtr pAuth)
{
}

void Player::send(uint32_t msgId, ::google::protobuf::MessageLite& refMsg)
{
	auto gate_session = g_gateSessionMgr->getGateSession(gate_session_fd_);
	if (gate_session) {
		gate_session->send(trans_id_, msgId, refMsg);
	}
}

void Player::sendGateLoginFail(uint32_t errorCode)
{
	PKG_LS_GG_Login_ACK resp;
	resp.set_result(PROTO_FAILURE);
	resp.set_error(errorCode);
	send(ProtoId::LS_GG_Login_ACK, resp);

	// change state to Logout
	changeState(FsmStateType::EFST_Logout);
}

void Player::sendGateLoginSucc()
{
	PKG_LS_GG_Login_ACK resp{};
	resp.set_result(PROTO_SUCCESS);
	resp.set_ptid(getPTID());
	resp.set_wplattype(0);// TODO loginInfo
	resp.set_abydatainfo("");// TODO loginData
	send(ProtoId::LS_GG_Login_ACK, resp);

	// change state to Online
	changeState(FsmStateType::EFST_Online);
}
