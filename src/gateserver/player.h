#pragma once

#include <string>
#include <variant>
#include <type_traits>

#include "networkEx/session.h"
#include "networkEx/connector.h"

#include "clientSession.h"
#include "playerState.h"

class Player {
public:
	Player() :fsm_{ this }
	{
	}

	Player(uint64_t id, std::string const& name)
		:id_{ id }, name_{ name }, fsm_{ this }
	{		
	}

	// function about player attribute like id/name and so on
public:
	inline uint64_t id()const { return id_; }
	inline std::string const& name()const { return name_; }
	bool changeName(std::string const& newName);

	// functions about i/o server like forward message to client/server
public:
	void forward2Login(uint32_t msgId, const char* data, uint16_t len, uint32_t fd);
	void forward2Server(uint32_t msgId, const char* data, uint16_t len, uint32_t fd);
	void forward2Client(uint32_t msgId, const char* data, uint16_t len);
	inline void setSession(ClientSessionPtr session) { session_ = session; }
	ClientSessionPtr getSession()const { return session_; }
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
	ClientSessionPtr session_{};
};

using PlayerPtr = std::shared_ptr<Player>;