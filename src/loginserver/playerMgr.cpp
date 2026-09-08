#include "playerMgr.h"

#include "share/utils/xtime.h"
#include "proto/commdef.pb.h"
using namespace commdefCmd;
#include "proto/gg_ls.pb.h"
using namespace GG_LS_Cmd;

PlayerID  PlayerMgr::player_idx_ = {};

PlayerMgr::~PlayerMgr()
{
}

SharedPlayerPtr PlayerMgr::fetchPlayer()
{
	auto sp = std::make_shared<Player>(++player_idx_);
	if (!sp)return nullptr;

	playerMap_.emplace(sp->playerID(),sp);
	return sp;
}

void PlayerMgr::releasePlayer(SharedPlayerPtr pPlayer)
{
	pTIDMap_.erase(pPlayer->getPTID());
	playerMap_.erase(pPlayer->playerID());
}

SharedPlayerPtr PlayerMgr::findPlayer(uint64_t playerID)
{
	auto it = playerMap_.find(playerID);
	if (it != playerMap_.end()) {
		return it->second;
	}
	return nullptr;
}

SharedPlayerPtr PlayerMgr::findPlayer(std::string const& ptid)
{
	auto playerID = getPlayerID(ptid);
	if (!playerID)return nullptr;
	
	return findPlayer(playerID);
}

void PlayerMgr::kickOffPlayer(std::string const& ptid)
{
	auto it = pTIDMap_.find(ptid);
	if (it != pTIDMap_.end()) {

		auto pPlayer = findPlayer(it->second);
		if (pPlayer) {
			PKG_LS_GG_Kickoff_NTF ntf;
			ntf.set_ptid(ptid);
			pPlayer->send(ProtoId::LS_GG_Kickoff_NTF, ntf);
			pPlayer->changeState(FsmStateType::EFST_Logout);
		}
	}
}

void PlayerMgr::addPtid2Map(PTID const& ptid, PlayerID playerID)
{
	pTIDMap_.emplace(ptid, playerID);
}

PlayerID PlayerMgr::getPlayerID(PTID const& ptid) const
{
	auto it = pTIDMap_.find(ptid);
	if (it != pTIDMap_.end()) {
		return it->second;
	}
	return 0;
}

void PlayerMgr::onUpdate()
{
	auto now = xtime::now();
	if (now - update_time_ >= 1000) {
		update_time_ = now;

		for (auto& it : playerMap_)
		{
			it.second->onUpdate();
		}
	}
}