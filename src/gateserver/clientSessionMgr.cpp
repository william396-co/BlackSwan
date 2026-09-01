#include "clientSessionMgr.h"

#include "share/networkEx/server.h"
#include "clientSession.h"
#include "player.h"
#include "playerCtrl.h"

void ClientSessionMgr::addSession(SessionPtr s, Connector* game_conn, Connector* login_conn)
{
	auto pPlayer = g_playerCtrl->fetchPlayer();
	auto pClientSession = std::make_shared<ClientSession>(s, game_conn, login_conn);
	if (pPlayer && pClientSession) {
		pClientSession->setPlayer(pPlayer.get());
		pPlayer->setSession(pClientSession);
		//pPlayer->changeState(LoginLs{});
		session_map_.emplace(pClientSession->fd(), pClientSession);
	}
}

void ClientSessionMgr::delSession(SessionPtr s)
{
	// TODO if need set player->Session to nullptr ? 
	// different level handle different logic ? 
	// High-level (player PlayerCtrl)
	// Low-Level (ClientSession ClientSessionMgr)
	auto it = session_map_.find(s->fd());
	if (it != session_map_.end()) {
		auto pPlayer = it->second->getPlayer();
		if (pPlayer) {
			pPlayer->setSession(nullptr);
		}
	}
	session_map_.erase(s->fd());
}
