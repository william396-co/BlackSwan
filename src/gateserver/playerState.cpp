#include "playerState.h"

#include "player.h"
#include "playerMgr.h"
#include "share/log/log.h"
#include "share/proto/cl_gg.pb.h"
using namespace ClientGateCmd;
#include "share/proto/gg_ls.pb.h"
using namespace GateLoginCmd;

#include "share/proto/commdefs.h"
#include "share/proto/errdefs.h"

#include <variant>
#include <utility>

void LoginLsState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool LoginLsState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	switch (event.msgID) {
	case GateLoginMsgID::GG_LS_Login_REQ:
		handleLoginReq(pPlayer, event);
		break;
	case GateLoginMsgID::LS_GG_Login_ACK:
		handleLoginAck(pPlayer, event);
		break;
	}
	return true;
}

void LoginLsState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void LoginLsState::handleLoginReq(PlayerPtr pPlayer, FsmEvent const& event)
{
	if (handled_ != Has_Handled_Nothing) {
		LOG_ERROR("{} LoginLs's sub-state was incorrect, ignore this req!", pPlayer->playerID());
		return;
	}

	auto erroCode = GGERR_NO_ERR;
	do
	{		
		auto areaGruop = event.pLoginReq->serverid();
		auto area = areaGruop / 1000;
		auto group = areaGruop % 1000;

		pPlayer->setClientVersion(event.pLoginReq->clientversion());
		pPlayer->setAuthenID(event.pLoginReq->authact());

		auto clientIp = pPlayer->getSession()->getClientIp();
		PKG_GG_LS_Login_REQ req;
		req.set_aptype(event.pLoginReq->aptype());
		req.set_clientversion(pPlayer->getClientVersion());
		req.set_authact(event.pLoginReq->authact());
		req.set_authstr(event.pLoginReq->authstr());
		req.set_invitecode(event.pLoginReq->invitecode());
		//req.set_ip(clientIp);
		req.set_reserve(event.pLoginReq->reserve());
		req.set_clienttype(event.pLoginReq->clienttype());
		req.set_areagroup(event.pLoginReq->serverid());

		pPlayer->getSession()->forward2Login(GG_LS_Login_REQ, req);

	} while (false);

	if (erroCode != GGERR_NO_ERR) {	
		PKG_GG_CLI_Login_ACK ack;
		ack.set_result(PROTO_FAILURE);
		ack.set_error(erroCode);
		event.playerSession_->forward2Client(ClientGateMsgID::GG_CLI_Login_ACK, ack);
		
		event.playerSession_->Close();
	}
}

void LoginLsState::handleLoginAck(PlayerPtr pPlayer, FsmEvent const& event)
{
}

void LoginDBState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool LoginDBState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginDBState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void ReselRoleState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool ReselRoleState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void ReselRoleState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}


void SelRoleState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool SelRoleState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void SelRoleState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void RoleOpState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool RoleOpState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void RoleOpState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void LoginGameState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool LoginGameState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginGameState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void InGameState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool InGameState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void InGameState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}
void LogoutGameState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool LogoutGameState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LogoutGameState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

void GlobalState::onEnter(PlayerPtr pPlayer)
{
	(void)pPlayer;
}

bool GlobalState::onEvent(PlayerPtr pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void GlobalState::onLeave(PlayerPtr pPlayer)
{
	(void)pPlayer;
}


void PlayerFSM::setState(PlayerState state)
{
	if (getFsmStateType(state) == FsmStateType::EFST_NULL)
		return;

	// previous = current;
	setPreviousState(current_state_);
	
	std::visit([player = owner_](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
	if constexpr (std::is_same_v<T, std::monostate>) {
		return;
	}
	else {
		return arg.onLeave(player);
	}
	},
		current_state_);

	// current = state
	setCurrentState(state);
	std::visit([player = owner_](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return;
		}
		else {
			return arg.onEnter(player);
		}},
		current_state_);
}

bool PlayerFSM::changeState(FsmStateType state)
{
	if (state < FsmStateType::EFST_LoginLs || state >= FsmStateType::EFST_Dummy) {
		LOG_ERROR("invalid State:{}", state);
		return false;
	}

	if (getFsmStateType(getCurrentState()) == state) {
		return false;
	}

	switch (state) {
	case FsmStateType::EFST_NULL:
		break;
	case FsmStateType::EFST_LoginLs:
		setState(LoginLsState{});
		break;
	case FsmStateType::EFST_LoginDB:
		setState(LoginDBState{});
		break;
	case FsmStateType::EFST_RoleOp:
		setState(RoleOpState{});
		break;
	case FsmStateType::EFST_SelRole:
		setState(SelRoleState{});
		break;
	case FsmStateType::EFST_LoginGs:
		setState(LoginGameState{});
		break;
	case FsmStateType::EFST_InGame:
		setState(InGameState{});
		break;
	case FsmStateType::EFST_ReselRole:
		setState(ReselRoleState{});
		break;
	case FsmStateType::EFST_Logout:
        setState(LogoutGameState{});
        break;
	case FsmStateType::EFST_Destroy:
		notifyOwnerDestroy();
		break;
	case FsmStateType::EFST_Dummy:
	default:
		setState(LoginLsState{});
    }
    return true;
}

bool PlayerFSM::onEvent(FsmEvent const& event)
{
	if (event.isGlobalEvent) {
		std::visit([player = owner_, &event](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				return false;
			}
			else {
				return arg.onEvent(player, event);
			}},
			global_state_);
	}
	else {
		std::visit([player = owner_, &event](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				return false;
			}
			else {
				return arg.onEvent(player, event);
			}},
			current_state_);
	}
	return true;
}

void PlayerFSM::notifyOwnerDestroy()
{
	owner_->onDestroy();
	owner_ = {};
}