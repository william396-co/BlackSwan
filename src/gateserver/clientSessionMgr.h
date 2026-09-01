#pragma once

#include <unordered_map>

#include "utils/singleton.h"
#include "share/networkEx/session.h"
#include "share/networkEx/connector.h"

#include "clientSession.h"

class ClientSessionMgr: public Singleton<ClientSessionMgr>
{
	friend class Singleton<ClientSessionMgr>;

	using ClientSessionMap = std::unordered_map<uint32_t, ClientSessionPtr>;
private:
	ClientSessionMgr() = default;
public:
	void addSession(SessionPtr s, Connector* game_conn, Connector* login_conn);
	void delSession(SessionPtr s);
private:
	ClientSessionMap session_map_;
};

#define g_clientSessionMgr ClientSessionMgr::InstancePtr()