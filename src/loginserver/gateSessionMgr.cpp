#include "gateSessionMgr.h"
#include "gateSession.h"
#include "log/log.h"

void GateSessionMgr::addSession(SessionPtr session)
{
	LOG_INFO("fd:{}", session->fd());
	addGateSession(new GateSession(session));
}

void GateSessionMgr::addGateSession(GateSessionPtr session)
{
	LOG_INFO("fd:{}", session->fd());
	session_map_.emplace(session->fd(), session);
}

void GateSessionMgr::delSession(uint32_t fd)
{
	LOG_INFO("fd:{}", fd);
	auto it = session_map_.find(fd);
	if (it != session_map_.end()) {
		delete it->second;
		session_map_.erase(it);
	}
}

GateSessionPtr GateSessionMgr::getGateSession(uint32_t fd)
{
	auto it = session_map_.find(fd);
	if (it != session_map_.end()) {
		return it->second;
	}
	return nullptr;
}

GateSessionMgr::~GateSessionMgr() 
{
	for (auto& it : session_map_) {
		delete it.second;
	}
}
