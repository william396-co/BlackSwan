#include "playerState.h"

#include "player.h"
#include "log/log.h"

#include <variant>
#include <utility>

void LoginState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LoginState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginState::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void LoginState::HandleAPLoginReq(PlayerPtr pPlayer, FsmEvent const& event)
{
}

void LoginState::HandleAPSuccCallback(PlayerPtr pPlayer, FsmEvent const& event)
{
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

void LogoutState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LogoutState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LogoutState::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void GlobalState::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool GlobalState::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
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
		owner_->onDestroy();
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