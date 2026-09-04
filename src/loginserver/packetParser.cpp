#include "packetParser.h"

#include "player.h"
#include "playerCtrl.h"

#include "proto/protocol.h"
#include "share/utils/xtime.h"
#include "proto/gg_ls.pb.h"
#include "proto/commdef.pb.h"
#include "proto/errdefs.h"
#include "proto/errdefs.h"
#include "log/log.h"

#include "gateSession.h"
#include "gateSessionMgr.h"
#include "constdefs.h"
#include "config.h"

using namespace GG_LS_Cmd;
using namespace InnerCmd;
CmdMessage PacketParser::findCmdMessage(uint32_t msgId)
{
	auto it = cmd_message_map_.find(msgId);
	if (it != cmd_message_map_.end()) {
		return it->second;
	}
	return {};
}

void PacketParser::Init() 
{
	registerCommand(ProtoId::GG_LS_Login_REQ, new PKG_GG_LS_Login_REQ,&PacketParser::RecvGgLoginReq);
	registerCommand(ProtoId::GG_LS_Logoff_NTF, new PKG_GG_LS_Logoff_NTF, &PacketParser::OnLogoffNtf);
}

void PacketParser::registerCommand(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, MessageHandler handler)
{
	cmd_message_map_[msgId] = CmdMessage{ std::move(pMsg),std::move(handler) };
}

void PacketParser::onUpdate()
{
	MessageParamList current_list;
	// Swap message list
	{
		std::lock_guard lk(message_list_mtx_);
		current_list.swap(message_list_);
	}
	for (auto const& it : current_list) {
		processMsg(it);
	}
}

void PacketParser::pushMsg(MessageParam msgParam)
{
	std::lock_guard lk(message_list_mtx_);
	message_list_.push_back(std::move(msgParam));
}

void PacketParser::processMsg(MessageParam const& msgParam)
{
	auto it = cmd_message_map_.find(msgParam.msg_id);
	if (it == cmd_message_map_.end()) {
		LOG_ERROR("message Id:{} not resiger CmdMessage", msgParam.msg_id);
		return;
	}
	
	constexpr auto buf_size = 1024 * 64;
	static char buf[buf_size];
	if (it->second.pMessage_ && it->second.pMessage_->ParseFromArray(buf, buf_size)) {
		if (!it->second.handler_) {
			LOG_ERROR("message id:{} not register handler", msgParam.msg_id);
			return;
		}
		it->second.handler_(it->second.pMessage_, it->second.pMessage_->ByteSizeLong(), msgParam.gate_session_fd, msgParam.transID);
		it->second.pMessage_->Clear();
	}
	else {
		it->second.handler_(nullptr, 0, msgParam.gate_session_fd, msgParam.transID);
	}
}

void PacketParser::RecvGgLoginReq(const void* pData, size_t len, uint32_t gate_session_fd, uint32_t transID)
{
	(void)len;
	auto pReq = (PKG_GG_LS_Login_REQ*)pData;
	if (!pReq) {
		LOG_ERROR("failed can not parse proto");
		return;
	}

	auto pPlayer = g_playerCtrl->fetchPlayer();
	if (!pPlayer) {
		auto pGateSession = g_gateSessionMgr->getGateSession(gate_session_fd);
		if (pGateSession) {
			PKG_LS_GG_Login_ACK resp;
			resp.set_result(COMMErr_system_error);
			resp.set_error(LSERR_Sys_Failure);
			pGateSession->send(transID, ProtoId::LS_GG_Login_ACK, resp);
		}
		return;
	}

	pPlayer->setGateSessionFd(gate_session_fd);
	pPlayer->setTransID(transID);

	// Check ClientVersion
	if (pReq->clientversion() < g_Config->getClientVersion()) {
		pPlayer->sendGateLoginFail(LSERR_Old_Client);
		return;
	}

	// Init Login Data
	in_addr addr;
	addr.S_un.S_addr = pReq->ip();
	pPlayer->setClientIP(inet_ntoa(addr));
	pPlayer->setAuthenID(pReq->authact());
	pPlayer->setPwd(pReq->authstr());
	pPlayer->setTransID(transID);
	pPlayer->setClientVer(pReq->clientversion());
	pPlayer->setAreaGroup(pReq->areagroup());
	pPlayer->setClientType(pReq->clienttype());
	pPlayer->setApType(pReq->aptype());
	pPlayer->setInviteCode(pReq->invitecode());
	pPlayer->setReserve(pReq->reserve());

	FsmEvent event{};
	event.transID = transID;
	event.msgID = ProtoId::GG_LS_Login_REQ;
	event.gate_session_fd = gate_session_fd;
	event.isGlobalEvent = false;
	pPlayer->onEvent(event);
}

void PacketParser::OnLogoffNtf(const void* pData, size_t len, uint32_t gate_session_fd, uint32_t transID)
{
	(void)len;
	auto pReq = (PKG_GG_LS_Logoff_NTF*)pData;
	if (!pReq) {
		LOG_ERROR("failed can not parse proto");
		return;
	}

	auto pPlayer = g_playerCtrl->findPlayer(pReq->ptid());
	if (!pPlayer) {
		LOG_ERROR("player PTID:{} not found",pReq->ptid());
		return;
	}

	FsmEvent event;
	event.transID = transID;
	event.msgID = GG_LS_Logoff_NTF;
	event.isGlobalEvent = true;	
	event.globalEvtType = GlobalStateType::EGET_LogoutNtf;
	event.gate_session_fd = gate_session_fd;	
	pPlayer->onEvent(event);
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
			LOG_DEBUG("Session fd:{} reply GateServer PONG", session->fd());
			session->replyInnerPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			LOG_DEBUG("Session fd:{} received GateServer PONG", session->fd());
			continue;
		}
		g_packetParser->pushMsg(MessageParam{ pack.id, pack.transID,session->fd(),std::string_view(pack.data,pack.sz) });
	}
	return len;
}
