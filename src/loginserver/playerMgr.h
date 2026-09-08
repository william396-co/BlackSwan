#pragma once

#include <unordered_map>
#include <string>
#include <string_view>


#include "utils/singleton.h"
#include "networkEx/session.h"
#include "player.h"

using PlayerID = uint64_t;
using PTID = std::string;
class PlayerMgr : public Singleton<PlayerMgr> 
{
	friend class Singleton<PlayerMgr>;
	using PlayerID2PlayerMap = std::unordered_map<PlayerID, SharedPlayerPtr>; // id, PlayerPtr
	using PTID2PlayerIDMap = std::unordered_map<PTID, PlayerID>; // ptid,id
private:
	PlayerMgr() = default;
public:	
	~PlayerMgr();

	SharedPlayerPtr fetchPlayer();
	void releasePlayer(SharedPlayerPtr pPlayer);

	SharedPlayerPtr findPlayer(PlayerID playerID);
	SharedPlayerPtr findPlayer(PTID const& ptid);
	void kickOffPlayer(PTID const& ptid);

	void addPtid2Map(PTID const& ptid, PlayerID playerID);
public:
	void onUpdate();
private:
	PlayerID getPlayerID(PTID const& ptid)const;
private:
	PlayerID2PlayerMap playerMap_;
	PTID2PlayerIDMap pTIDMap_;
	time_t update_time_{};// 1 second

	static PlayerID player_idx_;
};

#define g_playerMgr PlayerMgr::InstancePtr()