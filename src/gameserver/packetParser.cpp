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

void PacketParser::handleMessage(uint32_t msgId, std::string_view data_view, SessionPtr gate_session,uint32_t transID)
{
	std::cout << "transID" << transID << " msgId:" << msgId << " data:" << data_view << " len:" << data_view.size() << "\n";

	auto pPlayer = g_playerCtrl->findPlayer(transID, gate_session->fd());
	if (!pPlayer) {// first login
		pPlayer = g_playerCtrl->addPlayer(transID, gate_session);
	}
	pPlayer->recv(msgId, data_view.data(), data_view.size());
}

size_t PacketParser::onRecvData(const char* data, size_t len, SessionPtr session) {
	const char* recv_buf = data;
	InnerPacket pack;
	while (len) {
		if (!decode_inner_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();
		if (pack.id == CS_HeartBeat_Req) {
			std::cout << "Session fd:" << session->fd() << " reply GateServer PONG\n";
			session->replyInnerPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			std::cout << "Session fd:" << session->fd() << " received GateServer PONG\n";
			continue;
		}
		handleMessage(pack.id, std::string_view(pack.data, pack.sz), session,pack.transID);
	}
	return len;
}
