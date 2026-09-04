#include "playerState.h"

#include "player.h"
#include "playerCtrl.h"
#include "apMgr.h"
#include "config.h"

#include "log/log.h"
#include "proto/gg_ls.pb.h"
#include "proto/commdef.pb.h"
using namespace InnerCmd;
using namespace GG_LS_Cmd;
#include "proto/commdefs.h"
#include "proto/errdefs.h"

#include <variant>
#include <utility>
#include <cassert>

void LoginState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LoginState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	assert(pPlayer);
	
	// handle AP callback
	if (event.isLsInternalCall) {		

		switch (event.callbackType) {
		case EFsmLsCallbackType::ELCT_AP_Error:
			pPlayer->sendGateLoginFail(event.ptErrorCb->error_);
			break;
		case EFsmLsCallbackType::ELCT_AP_Succ:
			HandleAPSuccCallback(pPlayer, event);
			break;
		default:
			break;
		}
		return true;
	}

	// handle Gate message
	switch (event.msgID) {
	case ProtoId::GG_LS_Login_REQ:
		HandleAPLoginReq(pPlayer, event);
		break;
	default:
		break;
	}
	return true;
}

void LoginState::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void LoginState::HandleAPLoginReq(PlayerPtr pPlayer, FsmEvent const& event)
{
	if (g_Config->IsLocalTest()) {

		auto strPTID = pPlayer->getPTID();

		if (strPTID.find("_") == std::string::npos) {
			char szPTID[64];
			snprintf(szPTID, sizeof(szPTID), "%d_%d_%s", pPlayer->getAreaGroup(), pPlayer->getApType(), pPlayer->getAuthenID().c_str());
			strPTID = szPTID;
		}

		g_playerCtrl->kickOffPlayer(strPTID);
		pPlayer->setPTID(strPTID);

		pPlayer->sendGateLoginSucc();
	}

	// TODO check whitePt
	if (g_Config->bOpenWhitePt()) {

		return;
	}
	g_apmgr->checkApLoginData(pPlayer);
}

void LoginState::HandleAPSuccCallback(PlayerPtr pPlayer, FsmEvent const& event)
{
	assert(pPlayer);

	// TODO
	pPlayer->setLoginData(event.ptSuccCb->auth_);
	pPlayer->setAuth(event.ptSuccCb->auth_);
	
	char szPTID[64];
	snprintf(szPTID, sizeof(szPTID), "%d_%d_%s", pPlayer->getAreaGroup(), pPlayer->getApType(), pPlayer->getAuthenID().c_str());

	g_playerCtrl->kickOffPlayer(szPTID);
	pPlayer->setPTID(szPTID);

	pPlayer->sendGateLoginSucc();
}

void OnlineState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool OnlineState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void OnlineState::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void GlobalState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool GlobalState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	if (!event.isGlobalEvent)return false;
	switch (event.globalEvtType) {
	case GlobalStateType::EGET_LogoutNtf:
	case GlobalStateType::EGET_Kickoff_Ls:
		pPlayer->changeState(FsmStateType::EFST_Logout);
		break;
	default:
		break;
	}
	return true;
}

void GlobalState::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}


void PlayerFSM::setState(PlayerState state)
{
	if (getFsmStateType(state) == FsmStateType::EFST_Login)return;

	setPreviousState(current_state_);
	std::visit([player = owner_](auto&& arg) {
		return arg.onLeave(player);},
		current_state_);

	setCurrentState(state);
	std::visit([player = owner_](auto&& arg) {
		return arg.onEnter(player);},
		current_state_);
}

void PlayerFSM::notifyOwnerDestroy()
{
	owner_->onDestroy();
	owner_ = {};
}

bool PlayerFSM::changeState(FsmStateType state)
{
	if (state < FsmStateType::EFST_Login || state >= FsmStateType::EFST_Dummy) {
		return false;
	}

	if (getFsmStateType(getCurrentState()) == state) {
		LOG_DEBUG("current state is just you want, do nothing");
		return false;
	}

    switch (state) {
    case FsmStateType::EFST_Login:
		setState(LoginState{});
        break;
    case FsmStateType::EFST_Online:
		setState(OnlineState{});
        break;
	case FsmStateType::EFST_Logout:
	case FsmStateType::EFST_Destroy:
		notifyOwnerDestroy();
        break;
	case FsmStateType::EFST_Dummy:
	default:
		setState(LoginState{});
        break;
    }
    return true;
}

bool PlayerFSM::onEvent(FsmEvent const& event)
{
	if (event.isGlobalEvent) {
		std::visit([player = owner_, &event](auto&& arg) {
			return arg.onEvent(player, event);},
			global_state_);
	}
	else {
		std::visit([player = owner_, &event](auto&& arg) {
			return arg.onEvent(player, event);},
			current_state_);
	}
	return true;
}