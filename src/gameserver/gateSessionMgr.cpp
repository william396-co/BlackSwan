#include "gateSessionMgr.h"

#include "gateSession.h"

void GateSessionMgr::addSession(SessionPtr session)
{
	std::cout << __FUNCTION__ << " fd: " << session->fd() << "\n";
	addGateSession(new GateSession(session));
}

void GateSessionMgr::addGateSession(GateSessionPtr session)
{
	std::cout << __FUNCTION__ << " fd: " << session->fd() << "\n";
	session_map_.emplace(session->fd(), session);
}

void GateSessionMgr::delSession(uint32_t fd)
{
	std::cout << __FUNCTION__ << " fd: " << fd << "\n";
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
