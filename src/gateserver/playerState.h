#pragma once

#include <variant>
#include <type_traits>

#include "clientSession.h"

enum class FsmStateType 
{
	St_Null,
	St_LoginLs,
	St_LoginDB,
	St_RoleOp,
	St_SelRole,
	St_LoginGs,
	St_InGame,
	St_JumpGs,
	St_ReselRole,
	St_Logout,

	st_Global,
	st_Max
};

enum class GlobalStateType
{

};

class Player;
struct FsmEvent
{
	uint32_t msgID{};
	uint32_t errorCode{};
	uint32_t transID{};
	bool isGlobalEvent{};

	ClientSessionPtr session_{};
	// TODO message use std::variant<>
};

class NoneState {

public:
	inline FsmStateType getType()const { return FsmStateType::St_Null; }
	void onEnter(Player* pPlayer) { (void)pPlayer; }
	bool onEvent(Player* pPlayer, FsmEvent const& event) { (void)pPlayer; (void)event; return true; }
	void onLeave(Player* pPlayer) { (void)pPlayer; }
};

// Login LoginServer
class LoginLs
{
public:
	inline FsmStateType getType()const { return FsmStateType::St_LoginLs; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

// Login DBServer
class LoginDB
{
public:
	inline FsmStateType getType()const { return FsmStateType::St_LoginDB; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

// Login GameServer
class LoginGame
{
public:
	inline FsmStateType getType()const { return FsmStateType::St_LoginGs; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

// Logout GameServer
class LogoutGame
{
public:
	inline FsmStateType getType()const { return FsmStateType::St_Logout; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};


// Global State
class GlobalState {
public:
	inline FsmStateType getType()const { return FsmStateType::st_Global; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

using PlayerState = std::variant<NoneState, LoginLs, LoginDB, LoginGame, LogoutGame, GlobalState>;

#if __cplusplus > 202306
// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
#else
// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };
#endif

inline FsmStateType getFsmStateType(PlayerState const& state)
{
	return std::visit([](auto&& arg) {return arg.getType();}, state);
}

class PlayerFSM
{
public:
	explicit PlayerFSM(Player* owner) 
		:owner_{ owner },
		current_state_(LoginLs{}),
		previous_state_(LoginLs{}),
		global_state_(GlobalState{})
	{
	}

	//void reset();
	bool changeState(FsmStateType state);
	FsmStateType getCurStateType()const { return getFsmStateType(getCurrentState()); }
	FsmStateType getPrevStateType()const { return getFsmStateType(getPreviousState()); }
	bool onEvent(FsmEvent const& event);
private:
	void setState(PlayerState state);
private:
	inline void setCurrentState(PlayerState state) {
		current_state_ = state;
	}
	inline PlayerState getCurrentState()const {
		return current_state_;
	}
	inline void setPreviousState(PlayerState state) {
		previous_state_ = state;
	}
	inline PlayerState getPreviousState()const {
		return previous_state_;
	}
	inline PlayerState getGlobalState()const {
		return global_state_;
	}
	inline void setGlobalState(PlayerState state) {
		global_state_ = state;
	}
private:
	Player* owner_{};

	PlayerState current_state_{};
	PlayerState previous_state_{};

	PlayerState global_state_{};
};