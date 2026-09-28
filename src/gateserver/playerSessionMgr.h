#pragma once

#include <unordered_map>

#include "utils/singleton.h"
#include "share/networkEx/session.h"
#include "share/networkEx/connector.h"

#include "playerSession.h"

class PlayerSessionMgr: public Singleton<PlayerSessionMgr>
{
	friend class Singleton<PlayerSessionMgr>;

	using PlayerSessionMap = std::unordered_map<uint32_t, PlayerSessionPtr>;
private:
	PlayerSessionMgr() = default;
public:
	void addSession(SessionPtr s, Connector* game_conn, Connector* login_conn);
	void delSession(uint32_t fd);
	PlayerSessionPtr getSession(uint32_t fd);
private:
	PlayerSessionMap session_map_;
};

#define g_playerSessionMgr PlayerSessionMgr::InstancePtr()