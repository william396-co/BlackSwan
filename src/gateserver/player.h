#pragma once

#include <string>
#include <variant>
#include <type_traits>
#include <memory>

#include "networkEx/session.h"
#include "networkEx/connector.h"
#include "log/log.h"

#include "playerSession.h"
#include "playerState.h"

class Player : public std::enable_shared_from_this<Player>
{
public:
	Player()
		:id_{}, name_{}, fsm_{ this }
	{
		LOG_DEBUG("id:{}", id_);
	}

	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }, fsm_{ this }
	{
		LOG_DEBUG("id:{}", id_);
	}

	void onUpdate();
	void onDestroy();
	// function about player attribute like id/name and so on
public:
	inline uint64_t playerID()const { return id_; }
	inline std::string const& playerName()const { return name_; }
	bool changeName(std::string const& newName);

	void setClientVersion(uint32_t ver) { clientVersion_ = ver; }
	uint32_t getClientVersion()const { return clientVersion_; }
	void setClientGroup(uint32_t group) { clientGroup_ = group; }
	void setAuthenID(std::string const& authenID) { authenID_ = authenID; }
private:
	uint32_t clientVersion_{};
	uint32_t clientGroup_{};
	std::string authenID_{};

	// functions about i/o server like forward message to client/server
public:
	void forward2Login(uint32_t msgId, const char* data, uint32_t len, TransID transID);
	void forward2Server(uint32_t msgId, const char* data, uint32_t len, TransID transID);
	void forward2Client(uint32_t msgId, const char* data, uint16_t len);
	inline void setSession(PlayerSessionPtr session) { player_session_ = session; }
	PlayerSessionPtr getSession()const { return player_session_; }
private:
	//void send(uint32_t msgId, const char* data, uint16_t len);

	// functions about playerState
public:
	bool changeState(FsmStateType state) { return fsm_.changeState(state); }
	FsmStateType getCurStateType()const { return fsm_.getCurStateType(); }
	FsmStateType getPrevStateType()const { return fsm_.getPrevStateType(); }
	bool onEvent(FsmEvent const& event) { return fsm_.onEvent(event); }
private:
	uint64_t id_;
	std::string name_;
	PlayerFSM fsm_;
private:
	PlayerSessionPtr player_session_{};
};