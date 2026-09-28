#include "packetParser.h"

#include <cassert>

#include "player.h"
#include "proto/protocol.h"

#include "proto/cl_gg.pb.h"
using namespace ClientGateCmd;


void PacketParser::Init() {

	registerHandler((uint32_t)MsgId::ECHO_RESP, &PacketParser::HandleEchoResp);
	registerHandler((uint32_t)ClientGateMsgID::GG_CLI_Login_ACK, &PacketParser::HandleLoginResp);
	registerHandler((uint32_t)ClientGateMsgID::CLI_GG_Logout_NTF, &PacketParser::HandleLogoutNtf);
	registerHandler((uint32_t)ClientGateMsgID::CLI_GG_GS_MSG, &PacketParser::HandleGsResp);
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

void PacketParser::handleMessage(uint32_t msgId, std::string_view data_view) 
{
	std::cout << __FUNCTION__ << " msgId:" << msgId << " data: [" << data_view.data() << "]  len:" << data_view.size() << "\n";

	auto pHandler = g_packetParser->findHandle(msgId);
	if (!pHandler) {
		std::cerr << "msgid: " << msgId << " not register handler\n";
		return;
	}

	if (msgId != ClientGateMsgID::GS_GG_CLI_MSG)// Player Self Message
	{
		auto selfPlayer = nullptr;
		pHandler(selfPlayer, data_view.data(), data_view.size());
	}
	else { // Message from GS, include broadcast for other player(like AOI)
		auto pPlayer = nullptr;
		pHandler(pPlayer, data_view.data(), data_view.size());
	}
}

size_t PacketParser::onRecvData(const char* data, size_t len, SessionPtr session)
{
	const char* recv_buf = data;
	Packet pack;	
	while (len) {
		if (!decode_packet(recv_buf, len, pack)) {
			break;
		}
		len -= pack.size();
		recv_buf += pack.size();

		if (pack.id == CS_HeartBeat_Req) {
			std::cout << "Player reply GateServer PONG\n";
			session->replyPing();			
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			std::cout << "Player received GateServer PONG\n";
			continue;
		}
		handleMessage(pack.id, std::string_view(pack.data, pack.sz));
	}
	return len;
}

/////////////////////////////////////////////////////////////////////////////////
//  
// 
/////////////////////////////////////////////////////////////////////////////////
void PacketParser::HandleEchoResp(Player* pPlayer, const char* data, size_t len)
{
	(void)pPlayer;
	std::cout << __FUNCTION__ << " data : [" << data << "]  size:" << len << "\n";
}

void PacketParser::HandleLoginResp(Player* pPlayer, const char* data, size_t len)
{
	(void)pPlayer;
	(void)data;
	(void)len;
}

void PacketParser::HandleLogoutNtf(Player* pPlayer, const char* data, size_t len)
{
	(void)pPlayer;
	(void)data;
	(void)len;
}

void PacketParser::HandleGsResp(Player* pPlayer, const char* data, size_t len)
{
	(void)pPlayer;
	(void)data;
	(void)len;
}
