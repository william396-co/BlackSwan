#pragma once

#include <memory>

#include "networkEx/session.h"
#include "log/log.h"

#include <google/protobuf/message_lite.h>

constexpr auto MAX_SEND_PACKET_LEN = 64 * 1024 - InnerMsgHeaderSize;    //发送包大小

class GateSession {
public:
	explicit GateSession(SessionPtr s)
		: session_{ s }
	{
		LOG_DEBUG("fd:{}", fd());
	}
	~GateSession() 
	{
		LOG_DEBUG("fd:{}", fd());
	}

	inline uint32_t fd()const { return session_ ? session_->fd() : 0; }	

	void send(std::string msg);
	void send(uint32_t transID, uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
private:
	SessionPtr session_{};
};

using GateSessionPtr = GateSession*;