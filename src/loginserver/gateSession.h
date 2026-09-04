#pragma once

#include <memory>
#include <iostream>
#include "share/networkEx/session.h"

#include <google/protobuf/message_lite.h>

constexpr auto MAX_SEND_PACKET_LEN = 64 * 1024 - InnerMsgHeaderSize;    //发送包大小

class GateSession {
public:
	GateSession(SessionPtr s)
		: session_{ s }
	{
		std::cout << __FUNCTION__ << " fd:" << session_->fd() << "\n";
	}
	~GateSession() {
		std::cout << __FUNCTION__ << " fd:" << session_->fd() << "\n";
	}

	inline uint32_t fd()const { return session_ ? session_->fd() : 0; }	

	void send(std::string msg);
	void send(uint32_t transID, uint32_t msgId, ::google::protobuf::MessageLite& refMsg);
private:
	SessionPtr session_{};
	char m_szBuf_[MAX_SEND_PACKET_LEN];
};

using GateSessionPtr = GateSession*;