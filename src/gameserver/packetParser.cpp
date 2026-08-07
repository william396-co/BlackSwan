#include "packetParser.h"

#include "player.h"
#include "playerCtrl.h"


void PacketParser::registerHandler(uint32_t msgId, MessageHandler handler) 
{
	handleMap_.emplace(msgId, std::move(handler));
}

MessageHandler PacketParser::findHandle(uint32_t msgId) {
	auto it = handleMap_.find(msgId);
	if (it != handleMap_.end()) {
		return it->second;
	}
	return nullptr;
}

void PacketParser::handleMessage(uint32_t msgId, std::string_view data_view, SessionPtr session) {

	(void)session;
	// TODO  message decode playerId 
	auto playerId = 1;
	auto pPlayer = g_playerCtrl->findPlayer(playerId);
	if (!pPlayer) {
		std::cerr << "PlayerId:" << playerId << " not online\n";
		return;
	}
	auto pHandler = g_packetParser->findHandle(msgId);
	if (pHandler) {
		pHandler(data_view.data(), data_view.size(), pPlayer);
	}
	else {
		std::cerr << "msgid: " << msgId << " not register handler\n";
	}
}

size_t PacketParser::onRecvData(const char* data, size_t len, SessionPtr session) {
	const char* recv_buf = data;
	while (len) {
		DecodePacket pack{};
		if (!decode_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();
		handleMessage(pack.id, std::string_view(pack.data, pack.sz), session);
	}
	return len;
}