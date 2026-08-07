#include "playerCtrl.h"

void PlayerCtrl::addPlayer(uint64_t id, std::string const& name, PlayerPtr pPlayer)
{
	playerIdMap_.emplace(id, pPlayer);
	playerNameMap_.emplace(name, pPlayer);
}

PlayerPtr PlayerCtrl::findPlayer(uint64_t id)
{
	auto it = playerIdMap_.find(id);
	if (it != playerIdMap_.end()) {
		return it->second;
	}
	return nullptr;
}

PlayerPtr PlayerCtrl::findPlayer(std::string_view name)
{
	auto it = playerNameMap_.find(name.data());
	if (it != playerNameMap_.end()) {
		return it->second;
	}
	return nullptr;
}

void PlayerCtrl::delPlayer(uint64_t id)
{
	auto pPlayer = findPlayer(id);
	if (pPlayer) {
		delPlayer(pPlayer->name());
		delPlayer(id);
	}
}

void PlayerCtrl::delPlayer(std::string_view name)
{
	auto pPlayer = findPlayer(name);
	if (pPlayer) {
		delPlayer(pPlayer->id());
		delPlayer(name);
	}
}
