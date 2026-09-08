#include "packetParser.h"

#include "player.h"
#include "playerMgr.h"
#include "proto/protocol.h"
#include "proto/cl_gg.pb.h"
using namespace ClientGateCmd;

#include "proto/commdefs.h"

#include "playerSession.h"
#include "playerSessionMgr.h"

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

void PacketParser::handleClientPacket(uint32_t msgId, std::string_view data_view, SessionPtr session)
{
	if (msgId < CLI_GG_MSG_ID_MIN ||
		msgId > CLI_GG_MSG_ID_MAX)
	{
		session->stop();
		return;
	}

	auto pPlayerSession = g_playerSessionMgr->getSession(session->fd());
	if (!pPlayerSession || !pPlayerSession->getPlayer())
	{
		g_playerMgr->delPlayer(pPlayerSession->getPlayer()->playerID());
		g_playerSessionMgr->delSession(session->fd());
		return;
	}

	// first message Id check
	if (pPlayerSession->getPlayer()->getCurStateType() == FsmStateType::EFST_LoginLs && msgId != CLI_GG_Login_REQ) {
		g_playerSessionMgr->delSession(session->fd());
		return;
	}

	if (msgId != ClientGateMsgID::CLI_GG_GS_MSG)
	{
		auto pHandler = g_packetParser->findHandle(msgId);
		if (pHandler) {
			pHandler(data_view.data(), data_view.size(), pPlayerSession);
		}
	}
	else {// forward to Server
		//forward2Server(msgId, data_view, session);
		pPlayerSession->getPlayer()->forward2Server(msgId, data_view.data(), data_view.size(), session->fd());		
	}
}

void PacketParser::forward2Client(uint32_t msgId, std::string_view data_view, SessionPtr session, uint32_t transID)
{
	(void)session;

	auto pPlayer = g_playerMgr->findPlayer(transID);
	if (!pPlayer) {
		std::cerr << "Player fd:" << transID << " not in this gate\n";
		return;
	}
	pPlayer->forward2Client(msgId, data_view.data(), data_view.size());
}

void PacketParser::forward2Server(uint32_t msgId, std::string_view data_view, SessionPtr session)
{
	auto pPlayer = g_playerMgr->findPlayer(session->fd());
	if (!pPlayer) {
		std::cerr << "Player fd:" << session->fd() << " not in this gate\n";
		return;
	}
	pPlayer->forward2Server(msgId, data_view.data(), data_view.size(), session->fd());
}

void PacketParser::forward2Login(uint32_t msgId, std::string_view data_view, SessionPtr session)
{
	auto pPlayer = g_playerMgr->findPlayer(session->fd());
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
		handleClientPacket(pack.id, std::string_view(pack.data, pack.sz), session);
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
