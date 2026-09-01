#include "playerCtrl.h"

#include "share/utils/xtime.h"

uint64_t  PlayerCtrl::player_idx_ = {};

PlayerCtrl::~PlayerCtrl()
{
	for (auto& it : playerIdMap_) {
		delete it.second;
	}
}

PlayerPtr PlayerCtrl::fetchPlayer()
{
	auto p = new Player(++player_idx_, "");
	if (!p)return nullptr;

	playerIdMap_.emplace(p->id(), p);
	return p;
}

void PlayerCtrl::releasePlayer(PlayerPtr pPlayer)
{
	ptid2IdMap_.erase(pPlayer->getPTID());
	playerIdMap_.erase(pPlayer->id());
}

PlayerPtr PlayerCtrl::findPlayer(uint64_t id)
{
	auto it = playerIdMap_.find(id);
	if (it != playerIdMap_.end()) {
		return it->second;
	}
	return nullptr;
}

PlayerPtr PlayerCtrl::findPlayer(std::string const& ptid_name)
{
	auto id = findId(ptid_name);
	if (!id)return nullptr;

	auto it = playerIdMap_.find(id);
	if (it == playerIdMap_.end()) {
		return nullptr;
	}

	return it->second;
}

void PlayerCtrl::addPtid2Map(std::string const& ptid_name, uint64_t id)
{
	ptid2IdMap_.emplace(ptid_name, id);
}

uint64_t PlayerCtrl::findId(std::string const& ptid_name) const
{
	auto it = ptid2IdMap_.find(ptid_name);
	if (it != ptid2IdMap_.end()) {
		return it->second;
	}
	return 0;
}

void PlayerCtrl::onUpdate()
{
	auto now = xtime::now();
	if (now - update_time_ >= 1000) {
		update_time_ = now;

		for (auto& it : playerIdMap_)
		{
			it.second->onUpdate();
		}
	}
}