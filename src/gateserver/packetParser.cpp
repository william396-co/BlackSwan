#include "packetParser.h"

#include "player.h"
#include "playerCtrl.h"

#include "proto/protocol.h"

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

#if 0
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
#endif

void PacketParser::forward2Client(uint32_t msgId, std::string_view data_view, SessionPtr session, uint32_t fd)
{
	(void)session;

	auto pPlayer = g_playerCtrl->findPlayer(fd);
	if (!pPlayer) {
		std::cerr << "Player fd:" << fd << " not in this gate\n";
		return;
	}
	pPlayer->forward2Client(msgId, data_view.data(), data_view.size());
}

void PacketParser::forward2Server(uint32_t msgId, std::string_view data_view, SessionPtr session)
{
	auto pPlayer = g_playerCtrl->findPlayer(session->fd());
	if (!pPlayer) {
		std::cerr << "Player fd:" << session->fd() << " not in this gate\n";
		return;
	}
	pPlayer->forward2Server(msgId, data_view.data(), data_view.size(), session->fd());
}

size_t PacketParser::onRecvClientData(const char* data, size_t len, SessionPtr session) {
	const char* recv_buf = data;
	while (len) {
		Packet pack{};
		if (!decode_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();
		if (pack.id == CMD_PONG) {
			continue;
		}
		forward2Server(pack.id, std::string_view(pack.data, pack.sz), session);
	}
	return len;
}


size_t PacketParser::onRecvServerData(const char* data, size_t len, SessionPtr session)
{
	const char* recv_buf = data;
	while (len) {
		NetPacket pack{};
		if (!decode_net_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();

		if (pack.id == CMD_PING) {
			session->send(encode_net_packet(CMD_PONG, "PONG", sizeof("PONG"), 0));
			std::cout << "session fd:" << session->fd() << " Send PONG\n";
			continue;
		}
		forward2Client(pack.id, std::string_view(pack.data, pack.sz), session, pack.fd);
	}
	return len;
}