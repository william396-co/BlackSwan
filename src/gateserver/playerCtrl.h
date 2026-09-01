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
	using PlayerIDMap = std::unordered_map<uint64_t, PlayerPtr>;
	using PlayerNameMap = std::unordered_map<std::string, PlayerPtr>;

private:
	PlayerCtrl() = default;
public:	

	PlayerPtr fetchPlayer();

	PlayerPtr findPlayer(uint64_t id);
	PlayerPtr findPlayer(std::string_view name);
	void delPlayer(uint64_t id);
	void delPlayer(std::string_view name);

private:
	PlayerIDMap playerIdMap_;
	PlayerNameMap playerNameMap_;

	static uint64_t player_idx_;
};

#define g_playerCtrl PlayerCtrl::InstancePtr()