#pragma once

#include <iostream>

#include "share/networkEx/session.h"

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

private:
	SessionPtr session_{};
};

using GateSessionPtr = GateSession*;