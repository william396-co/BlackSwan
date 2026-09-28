#include "playerMgr.h"
#include "player.h"

uint64_t  PlayerMgr::player_idx_ = {};

SharedPlayerPtr PlayerMgr::fetchPlayer()
{
	auto p = std::make_shared<Player>(++player_idx_, "");
	if (p) {
		playerIdMap_.emplace(p->playerID(), p);
	}
	return p;
}

SharedPlayerPtr PlayerMgr::findPlayer(PlayerID playerID)
{
	auto it = playerIdMap_.find(playerID);
	if (it != playerIdMap_.end()) {
		return it->second;
	}
	return nullptr;
}

SharedPlayerPtr PlayerMgr::findPlayer(PlayerName playerName)
{
	auto it = playerNameMap_.find(playerName);
	if (it != playerNameMap_.end()) {
		return it->second;
	}
	return nullptr;
}

void PlayerMgr::delPlayer(PlayerID playerID)
{
	auto pPlayer = findPlayer(playerID);
	if (pPlayer) {
		delPlayer(pPlayer->playerName());
		delPlayer(playerID);
	}
}

void PlayerMgr::delPlayer(PlayerName playerName)
{
	auto pPlayer = findPlayer(playerName);
	if (pPlayer) {
		delPlayer(pPlayer->playerID());
		delPlayer(playerName);
	}
}

void PlayerMgr::onUpdate() 
{
	for (auto& it : playerIdMap_) 
	{
		it.second->onUpdate();
	}
}

void PlayerMgr::releasePlayer(SharedPlayerPtr pPlayer)
{
	playerIdMap_.erase(pPlayer->playerID());
	playerNameMap_.erase(pPlayer->playerName());
}