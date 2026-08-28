#include "playerCtrl.h"


constexpr uint64_t makeKey(uint32_t client_fd, uint32_t gate_fd)
{
	return (uint64_t)client_fd << 32U | (uint64_t)gate_fd;
}

PlayerPtr PlayerCtrl::addPlayer(uint32_t clientId, SessionPtr gateSession)
{
	auto pPlayer = std::make_shared<Player>(clientId, gateSession);
	addPlayer(makeKey(clientId, gateSession->fd()), "", pPlayer);
	return pPlayer;
}

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

PlayerPtr PlayerCtrl::findPlayer(uint32_t clientId, uint32_t gate_fd)
{
	auto it = playerFdMap_.find(makeKey(clientId, gate_fd));
	if (it != playerFdMap_.end()) {
		return it->second;
	}
	return nullptr;
}

void PlayerCtrl::delPlayer(uint64_t id)
{
	auto pPlayer = findPlayer(id);
	if (pPlayer) {
		delPlayer(pPlayer->name());
		delPlayer(pPlayer->client_fd());
	}
	playerIdMap_.erase(id);
}

void PlayerCtrl::delPlayer(std::string_view name)
{
	auto pPlayer = findPlayer(name);
	if (pPlayer) {
		delPlayer(pPlayer->id());		
		delPlayer(pPlayer->client_fd());
	}
	playerNameMap_.erase(name.data());
}

void PlayerCtrl::delPlayer(uint32_t clientId)
{
	auto pPlayer = findPlayer(clientId);
	if (pPlayer) {
		delPlayer(pPlayer->id());
		delPlayer(pPlayer->name());
	}
	playerFdMap_.erase(clientId);
}
