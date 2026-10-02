#include "playerMgr.h"


PlayerPtr PlayerMgr::findPlayer(uint64_t playerId) {
	auto it = player_map_.find(playerId);
	if (it != player_map_.end()) {
		return it->second;
	}
	return {};
}

PlayerPtr PlayerMgr::fetchPlayer() 
{
	auto pPlayer = std::make_shared<Player>();
	player_map_.emplace(pPlayer->getPlayerID(), pPlayer);
	return pPlayer;
}