#pragma once

#include <unordered_map>
#include <memory>

#include "utils/singleton.h"
#include "gateSession.h"

class GateSessionMgr : public Singleton<GateSessionMgr> 
{
	using GateSessionMap = std::unordered_map<uint32_t, GateSessionPtr>;
	friend class Singleton<GateSessionMgr>;
	GateSessionMgr() = default;
public:
	~GateSessionMgr();

	void addSession(SessionPtr session);
	void delSession(uint32_t fd);
	GateSessionPtr getGateSession(uint32_t fd);
private:
	void addGateSession(GateSessionPtr session);
private:	
	GateSessionMap session_map_;
};

#define g_gateSessionMgr GateSessionMgr::InstancePtr()