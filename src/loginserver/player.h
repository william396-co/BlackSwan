#pragma once

#include <string>
#include <variant>
#include <type_traits>

#include "networkEx/session.h"
#include "networkEx/connector.h"

#include "gateSession.h"
#include "playerState.h"

class Player
{
public:
	Player() :fsm_{ this }
	{
	}

	Player(uint64_t id, std::string const& ptid_name)
		:id_{ id }, ptid_name_{ ptid_name }, fsm_{ this }
	{
		std::cout << __FUNCTION__ << " id:" << id_ << "\n";
	}
	~Player() {
		std::cout << __FUNCTION__ << " id:" << id_ << "\n";
	}
	void onDestroy();
	void onUpdate();

	// function about player attribute like id/name and so on
public:
	inline uint64_t id()const { return id_; }
	inline std::string const& getPTID()const { return ptid_name_; }
	void setPTID(std::string const& ptid_name);

	// functions about i/o server like forward message to client/server
public:
	inline void setGateFd(uint32_t fd) { gate_fd_ = fd; }
private:
	void send(uint32_t msgId, const char* data, uint16_t len);

	// functions about playerState
public:
	bool changeState(FsmStateType state) { return fsm_.changeState(state); }
	FsmStateType getCurStateType()const { return fsm_.getCurStateType(); }
	FsmStateType getPrevStateType()const { return fsm_.getPrevStateType(); }
	bool onEvent(FsmEvent const& event) { return fsm_.onEvent(event); }
private:
	uint64_t id_;
	std::string ptid_name_;
	PlayerFSM fsm_;
	uint32_t gate_fd_;
	uint32_t client_fd_;
};

using PlayerPtr = Player*;