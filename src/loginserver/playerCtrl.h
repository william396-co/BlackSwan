#pragma once

#include <unordered_map>
#include <string>
#include <string_view>


#include "utils/singleton.h"
#include "networkEx/session.h"

#include "player.h"


class PlayerCtrl : public Singleton<PlayerCtrl> 
{
	friend class Singleton<PlayerCtrl>;
	using PlayerIDMap = std::unordered_map<uint64_t, PlayerPtr>; // id, PlayerPtr
	using Ptid2IDMap = std::unordered_map<std::string, uint64_t>; // ptid,id
private:
	PlayerCtrl() = default;
public:	
	~PlayerCtrl();

	PlayerPtr fetchPlayer();
	void releasePlayer(PlayerPtr pPlayer);

	PlayerPtr findPlayer(uint64_t id);
	PlayerPtr findPlayer(std::string const& ptid_name);

	void addPtid2Map(std::string const& ptid_name,uint64_t id);
public:
	void onUpdate();
private:
	uint64_t findId(std::string const& ptid_name)const;
private:
	PlayerIDMap playerIdMap_;
	Ptid2IDMap ptid2IdMap_;
	time_t update_time_{};// 1 second

	static uint64_t player_idx_;
};

#define g_playerCtrl PlayerCtrl::InstancePtr()