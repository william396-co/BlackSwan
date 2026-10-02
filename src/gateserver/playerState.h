#pragma once

#include <variant>
#include <type_traits>
#include <utility>

#include "playerSession.h"

#include "share/proto/gg_ls.pb.h"
using namespace GateLoginCmd;
#include "share/proto/cl_gg.pb.h"
using namespace ClientGateCmd;


enum class FsmStateType 
{
	EFST_NULL,
	EFST_LoginLs,
	EFST_LoginDB,
	EFST_RoleOp,
	EFST_SelRole,
	EFST_LoginGs,
	EFST_InGame,
	EFST_JumpGs,
	EFST_ReselRole,
	EFST_Logout,

	EFST_Global,
	EFST_Destroy,

	EFST_Dummy,
};

enum class EFsmGlobalEvtType
{
    EGET_Timeout,
    EGET_LogoutNtf,
	EGET_OfflineTrusteeNtf,

    EGET_Kickoff_Ls,
    EGET_Kickoff_Db,
    EGET_Kickoff_Gs,
    EGET_Kickoff_Gg,

    EGET_Client_Socket_Close,
    
    EGET_Normal,
};

class Player;
using PlayerPtr = Player*;

class PlayerSession;
struct FsmEvent
{
	uint32_t msgID{};
	uint32_t errorCode{};
	TransID transID{};
	bool isGlobalEvent{};

	union {
		EFsmGlobalEvtType    globalEvtType;
	};
	PlayerSessionPtr playerSession_{};
	//ClientSessionPtr session_{};
	// TODO message use std::variant<>

	union {
		PKG_CLI_GG_Login_REQ* pLoginReq;
		PKG_LS_GG_Login_ACK* pLsLoginAck;
		PKG_LS_GG_Kickoff_NTF* pLsKickNtf;
	};
};

// Login LoginServer
class LoginLsState
{
	enum {
		Has_Handled_Nothing,
		Has_Handled_LoginReq,
		Has_Handled_LoginAck,
	};
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_LoginLs; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
private:
	void handleLoginReq(PlayerPtr pPlayer, FsmEvent const& event);
	void handleLoginAck(PlayerPtr pPlayer, FsmEvent const& event);
private:
	int32_t handled_{ Has_Handled_Nothing };
};

// Login DBServer
class LoginDBState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_LoginDB; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// RoleOp State
class RoleOpState 
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_RoleOp; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// SelRole State
class SelRoleState {
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_SelRole; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// ReselRole State
class ReselRoleState 
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_ReselRole; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// Login GameServer
class LoginGameState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_LoginGs; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// InGame State
class InGameState 
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_InGame; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

// Logout GameServer
class LogoutGameState
{
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Logout; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};


// Global State
class GlobalState {
public:
	inline FsmStateType getType()const { return FsmStateType::EFST_Global; }
	void onEnter(PlayerPtr pPlayer);
	bool onEvent(PlayerPtr pPlayer, FsmEvent const& event);
	void onLeave(PlayerPtr pPlayer);
};

using PlayerState = std::variant<std::monostate, LoginLsState, LoginDBState, LoginGameState,InGameState,
	LogoutGameState, RoleOpState, SelRoleState, ReselRoleState, GlobalState>;

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
	return std::visit([](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				return FsmStateType::EFST_NULL;
			}
			else {
				return arg.getType();
			}
		}, state);
}

class PlayerFSM
{
public:
	explicit PlayerFSM(PlayerPtr owner)
		:owner_{ owner },
		current_state_(LoginLsState{}),
		previous_state_(LoginLsState{}),
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
	PlayerPtr owner_{};

	PlayerState current_state_{};
	PlayerState previous_state_{};

	PlayerState global_state_{};
};