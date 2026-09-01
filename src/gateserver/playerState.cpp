#include "playerState.h"

#include "player.h"

#include <variant>
#include <utility>

void LoginLs::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LoginLs::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginLs::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void LoginDB::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LoginDB::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginDB::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void LoginGame::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LoginGame::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LoginGame::onLeave(Player* pPlayer)
{
	(void)pPlayer;
}

void LogoutGame::onEnter(Player* pPlayer)
{
	(void)pPlayer;
}

bool LogoutGame::onEvent(Player* pPlayer, FsmEvent const& event)
{
	(void)pPlayer;
	(void)event;
	return false;
}

void LogoutGame::onLeave(Player* pPlayer)
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
	if (getFsmStateType(state) == FsmStateType::St_Null)return;

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
	if (state <= FsmStateType::St_Null || state >= FsmStateType::st_Max) {
		return false;
	}

	if (getFsmStateType(getCurrentState()) == state) {
		return false;
	}

    switch (state) {
    case FsmStateType::St_Null:
        break;
    case FsmStateType::St_LoginLs:
        setState(LoginLs{});
        break;
    case FsmStateType::St_LoginGs:
        setState(LoginGame{});
        break;
    case FsmStateType::St_LoginDB:
        setState(LoginDB{});
        break;
    case FsmStateType::St_Logout:
        setState(LogoutGame{});
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