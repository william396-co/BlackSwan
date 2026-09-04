#pragma once

#include <variant>
#include <type_traits>

#include "gateSession.h"
#include "proto/gg_ls.pb.h"
#include "authInfo.h"

using namespace GG_LS_Cmd;

enum class FsmStateType
{
	EFST_Login,
	EFST_Online,
	EFST_Logout,

	EFST_Global,
	EFST_Destroy,

	EFST_Dummy
};

enum class GlobalStateType
{
	EGET_Timeout,
	EGET_LogoutNtf,
	EGET_Kickoff_Ls,

	EGET_Normal,
};

enum class EFsmLsCallbackType
{
	ELCT_AP_Error,
	ELCT_AP_Succ,

	ELCT_MYCHECK_RET,

	EGCT_Dummy
};

struct PT_LS_Error_Callback 
{
	AuthInfoPtr auth_{};
	int error_{};
};
struct PT_LS_Succ_Callback 
{
	AuthInfoPtr auth_{};
};

class Player;
using PlayerPtr = Player*;
struct FsmEvent
{
	uint32_t msgID{};
	uint32_t errorCode{};
	uint32_t transID{};// Transparent ID
	bool isGlobalEvent{};
	bool isLsInternalCall{};

	union
	{
		GlobalStateType  globalEvtType;
		EFsmLsCallbackType callbackType;
	};
	union {
		PT_LS_Error_Callback* ptErrorCb;
		PT_LS_Succ_Callback* ptSuccCb;
	};

	uint32_t gate_session_fd{};
};

class NoneState {

public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Dummy; }
	void onEnter(Player* pPlayer) { (void)pPlayer; }
	bool onEvent(Player* pPlayer, FsmEvent const& event) { (void)pPlayer; (void)event; return true; }
	void onLeave(Player* pPlayer) { (void)pPlayer; }
};

// Login
class LoginState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Login; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);

	void HandleAPLoginReq(PlayerPtr pPlayer, FsmEvent const& event);
	void HandleAPSuccCallback(PlayerPtr pPlayer, FsmEvent const& event);
};

// Online
class OnlineState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Online; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

// Logout
class LogoutState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Logout; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

// Global State
class GlobalState {
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Global; }
	void onEnter(Player* pPlayer);
	bool onEvent(Player* pPlayer, FsmEvent const& event);
	void onLeave(Player* pPlayer);
};

using PlayerState = std::variant<NoneState, LoginState, OnlineState, GlobalState>;

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
		current_state_(LoginState{}),
		previous_state_(LoginState{}),
		global_state_(GlobalState{})
	{
	}

	bool changeState(FsmStateType state);
	FsmStateType getCurStateType()const { return getFsmStateType(getCurrentState()); }
	FsmStateType getPrevStateType()const { return getFsmStateType(getPreviousState()); }
	bool onEvent(FsmEvent const& event);	
private:
	void setState(PlayerState state);
	void notifyOwnerDestroy();
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
