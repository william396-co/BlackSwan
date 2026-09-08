#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <memory>


#include "utils/singleton.h"
#include "networkEx/session.h"

class Player;
using SharedPlayerPtr = std::shared_ptr<Player>;

using PlayerID = uint64_t;
using PlayerName = std::string;
class PlayerMgr : public Singleton<PlayerMgr> 
{
	friend class Singleton<PlayerMgr>;
	using PlayerIDMap = std::unordered_map<PlayerID, SharedPlayerPtr>;
	using PlayerNameMap = std::unordered_map<PlayerName, SharedPlayerPtr>;

private:
	PlayerMgr() = default;
public:
	void onUpdate();
public:
	SharedPlayerPtr fetchPlayer();
	void releasePlayer(SharedPlayerPtr pPlayer);

	SharedPlayerPtr findPlayer(PlayerID playerID);
	SharedPlayerPtr findPlayer(PlayerName playerName);
	void delPlayer(PlayerID playerID);
	void delPlayer(PlayerName playerName);

private:
	PlayerIDMap playerIdMap_;
	PlayerNameMap playerNameMap_;

	static uint64_t player_idx_;
};

#define g_playerMgr PlayerMgr::InstancePtr()