#pragma once

#include <unordered_map>

#include "utils/singleton.h"
#include "player.h"

class PlayerMgr : public Singleton<PlayerMgr>
{
	using PlayerMap = std::unordered_map<uint64_t, PlayerPtr>;
	friend class Singleton<PlayerMgr>;
	PlayerMgr() = default;
public:
	~PlayerMgr() = default;

	PlayerPtr findPlayer(uint64_t playerId);
	PlayerPtr fetchPlayer();
private:
	PlayerMap player_map_;
};

#define g_playerMgr PlayerMgr::InstancePtr()