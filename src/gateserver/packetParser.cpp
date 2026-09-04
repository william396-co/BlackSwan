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

void PacketParser::forward2Client(uint32_t msgId, std::string_view data_view, SessionPtr session, uint32_t transID)
{
	(void)session;

	auto pPlayer = g_playerCtrl->findPlayer(transID);
	if (!pPlayer) {
		std::cerr << "Player fd:" << transID << " not in this gate\n";
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

void PacketParser::forward2Login(uint32_t msgId, std::string_view data_view, SessionPtr session)
{
	auto pPlayer = g_playerCtrl->findPlayer(session->fd());
	if (!pPlayer) {
		std::cerr << "Player fd:" << session->fd() << " not in this gate\n";
		return;
	}
	pPlayer->forward2Login(msgId, data_view.data(), data_view.size(), session->fd());
}

size_t PacketParser::onRecvClientData(const char* data, size_t len, SessionPtr session) {
	const char* recv_buf = data;
	Packet pack;
	while (len) {
		if (!decode_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();
		if (pack.id == CS_HeartBeat_Req) {
			std::cout << "Session fd:" << session->fd() << " reply Client PONG\n";
			session->replyPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			std::cout << "Session fd:" << session->fd() << " received Client PONG\n";
			continue;
		}
		forward2Server(pack.id, std::string_view(pack.data, pack.sz), session);
	}
	return len;
}

size_t PacketParser::onRecvServerData(const char* data, size_t len, SessionPtr session)
{
	const char* recv_buf = data;
	InnerPacket pack;
	while (len) {
		if (!decode_inner_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();

		if (pack.id == CS_HeartBeat_Req) {
			std::cout << "Session fd:" << session->fd() << " reply GameServer PONG\n";
			session->replyInnerPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			std::cout << "Session fd:" << session->fd() << " received GameServer PONG\n";
			continue;
		}
		forward2Client(pack.id, std::string_view(pack.data, pack.sz), session, pack.transID);
	}
	return len;
}

size_t PacketParser::onRecvLoginData(const char* data, size_t len, SessionPtr session)
{
	const char* recv_buf = data;
	InnerPacket pack;
	while (len) {
		if (!decode_inner_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();

		if (pack.id == CS_HeartBeat_Req) {
			std::cout << "Session fd:" << session->fd() << " reply LoginServer PONG\n";
			session->replyInnerPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			std::cout << "Session fd:" << session->fd() << " received LoginServer PONG\n";
			continue;
		}
		forward2Client(pack.id, std::string_view(pack.data, pack.sz), session, pack.transID);
	}
	return len;
}
