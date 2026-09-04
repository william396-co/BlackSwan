#pragma once

#include <string>
#include <variant>
#include <type_traits>
#include <google/protobuf/message_lite.h>

#include "networkEx/session.h"
#include "networkEx/connector.h"
#include "share/log/log.h"

#include "gateSession.h"
#include "playerState.h"

class Player
{
public:
	explicit Player(uint64_t id)
		:fsm_{ this }, id_{ id }
	{		
		LOG_DEBUG("id:{}", id_);
	}
	~Player() {
		LOG_DEBUG("id:{}", id_);
	}
	void onDestroy();
	void onUpdate();

	// function about player attribute like id/name and so on
public:
	inline uint64_t id()const { return id_; }
	inline std::string const& getPTID()const { return szPTID_; }
	void setPTID(std::string const& szPTID);

	inline std::string const& getAuthenID()const { return szAuthenID_; }
	inline void setAuthenID(std::string const& szAuthenID) { szAuthenID_ = szAuthenID; }

	inline void setTransID(uint32_t transId) { trans_id_ = transId; }
	inline uint32_t getTransID()const { return trans_id_; }

	inline void setClientIP(std::string const& clientIP) { client_IP_ = clientIP; }
	inline std::string const& getClientIP()const { return client_IP_; }

	inline void setPwd(std::string const& pwd) { szPwd_ = pwd; }
	inline void setClientVer(uint32_t clientVer) { clientVersion_ = clientVer; }
	inline int getClientVer()const { return clientVersion_; }
	inline void setAreaGroup(uint32_t areaGroup) { areaGroup_ = areaGroup; }
	inline uint32_t getAreaGroup()const { return areaGroup_; }
	inline void setClientType(uint16_t clientType) { clientType_ = clientType; }
	inline void setApType(uint16_t apType) { apType_ = apType; }
	inline uint16_t getApType()const { return apType_; }
	inline void setGateSessionFd(uint32_t fd) { gate_session_fd_ = fd; }
	inline void setInviteCode(std::string const& code) { inviteCode_ = code; }
	inline void setReserve(uint32_t reserve) { reserve_ = reserve; }
	void setLoginData(AuthInfoPtr pAuth);
	void setAuth(AuthInfoPtr pAuth) { pAuth_ = pAuth; }
	// functions about i/o server like forward message to client/server
public:
	//inline void setGateSessionFd(uint32_t fd) { gate_session_fd_ = fd; }
	void send(uint32_t msgId, ::google::protobuf::MessageLite& refMsg);

	// Send Gate LoginFail
	void sendGateLoginFail(uint32_t errorCode);
	// Send Gate LoginSucc
	void sendGateLoginSucc();

	// functions about playerState
public:
	bool changeState(FsmStateType state) { return fsm_.changeState(state); }
	FsmStateType getCurStateType()const { return fsm_.getCurStateType(); }
	FsmStateType getPrevStateType()const { return fsm_.getPrevStateType(); }
	bool onEvent(FsmEvent const& event) { return fsm_.onEvent(event); }
private:
	PlayerFSM fsm_;
	uint32_t gate_session_fd_{};
	uint32_t client_fd_{};

private:
	uint64_t id_{};

	std::string szPTID_{};// verfied account
	std::string szAuthenID_{};//authened Id
	std::string szPwd_{};//authened pwd
	std::string client_IP_{};
	uint32_t clientVersion_{};
	uint32_t areaGroup_{};
	std::string inviteCode_{};
	uint32_t reserve_{};
	uint16_t clientType_{};
	uint16_t apType_{};
	uint32_t trans_id_{};
	AuthInfoPtr pAuth_{};
};

using PlayerPtr = Player*;