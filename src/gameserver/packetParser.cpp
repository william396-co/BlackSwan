#include "packetParser.h"

#include "player.h"
#include "playerCtrl.h"

#include "proto/protocol.h"


void PacketParser::Init() 
{
	registerHandler((uint32_t)MsgId::ECHO_REQ, &PacketParser::HandleEchoReq);
}

void PacketParser::registerHandler(uint32_t msgId, MessageHandler handler) 
{
	handleMap_.emplace(msgId, std::move(handler));
}

MessageHandler PacketParser::findHandle(uint32_t msgId) 
{
	auto it = handleMap_.find(msgId);
	if (it != handleMap_.end()) {
		return it->second;
	}
	return nullptr;
}

void PacketParser::HandleEchoReq(const char* data, size_t len, Player* pPlayer)
{
	std::cout << "Player recieve msg ["<< data << "]\n";
	// echo
	pPlayer->send((uint32_t)MsgId::ECHO_RESP, data, len);
}

void PacketParser::handleMessage(uint32_t msgId, std::string_view data_view, SessionPtr gate_session,uint32_t client_fd)
{
	std::cout << "client_fd" << client_fd << " msgId:" << msgId << " data:" << data_view << " len:" << data_view.size() << "\n";

	auto pPlayer = g_playerCtrl->findPlayer(client_fd, gate_session->fd());
	if (!pPlayer) {// first login
		pPlayer = g_playerCtrl->addPlayer(client_fd, gate_session);
	}
	pPlayer->recv(msgId, data_view.data(), data_view.size());
}

size_t PacketParser::onRecvData(const char* data, size_t len, SessionPtr session) {
	const char* recv_buf = data;
	while (len) {
		NetPacket pack{};
		if (!decode_net_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();
		if (pack.id == CMD_PING) {
			std::cout << "Session fd:" << session->fd() << " reply GateServer PONG\n";
			session->send(encode_net_packet(CMD_PONG, "PONG", sizeof("PONG"), 0));
			continue;
		}
		if (pack.id == CMD_PONG) {
			std::cout << "Session fd:" << session->fd() << " received GateServer PONG\n";
			continue;
		}
		handleMessage(pack.id, std::string_view(pack.data, pack.sz), session,pack.fd);
	}
	return len;
}
