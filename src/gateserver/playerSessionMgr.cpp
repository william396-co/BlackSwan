#include "playerSessionMgr.h"

#include "playerSession.h"
#include "player.h"
#include "playerMgr.h"

void PlayerSessionMgr::addSession(SessionPtr s, Connector* game_conn, Connector* login_conn)
{
	auto pPlayer = g_playerMgr->fetchPlayer();
	if (!pPlayer)return;
	auto pPlayerSession = std::make_shared<PlayerSession>(s, game_conn, login_conn);
	if (!pPlayerSession) {
		return;
	}
	pPlayerSession->setPlayer(pPlayer.get());
	pPlayer->setSession(pPlayerSession);
	pPlayer->changeState(FsmStateType::EFST_LoginLs);
	session_map_.emplace(pPlayerSession->fd(), pPlayerSession);
}

void PlayerSessionMgr::delSession(uint32_t fd)
{
	// TODO if need set player->Session to nullptr ? 
	// different level handle different logic ? 
	// High-level (player PlayerCtrl)
	// Low-Level (ClientSession ClientSessionMgr)
	auto it = session_map_.find(fd);
	if (it != session_map_.end()) {
		auto pPlayer = it->second->getPlayer();
		if (pPlayer) {
			pPlayer->setSession(nullptr);
		}
	}
	session_map_.erase(fd);
}

PlayerSessionPtr PlayerSessionMgr::getSession(uint32_t fd)
{
	auto it = session_map_.find(fd);
	if (it != session_map_.end()) {
		return it->second;
	}
	return {};
}
