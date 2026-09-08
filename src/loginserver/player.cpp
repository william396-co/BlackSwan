#include "player.h"

#include "networkEx/packet.h"
#include "gateSession.h"
#include "gateSessionMgr.h"
#include "playerMgr.h"
#include "proto/gg_ls.pb.h"
#include "proto/commdef.pb.h"
using namespace InnerCmd;
using namespace GG_LS_Cmd;
#include "proto/commdefs.h"

void Player::onDestroy()
{
	g_playerMgr->releasePlayer(shared_from_this());
}

void Player::onUpdate()
{
	// running on main-thread
	if (getCurStateType() == FsmStateType::EFST_Login)
		return;

	// TODO timeout check
}

void Player::setPTID(std::string const& szPTID)
{
	szPTID_ = szPTID;
	g_playerMgr->addPtid2Map(szPTID_, playerID());
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
