#include "packetParser.h"

#include "player.h"
#include "playerCtrl.h"

#include "proto/protocol.h"
#include "share/utils/xtime.h"
#include "proto/gg_ls.pb.h"
#include "proto/commdef.pb.h"
#include "log/log.h"

#include "gateSession.h"
#include "gateSessionMgr.h"
#include "constdefs.h"

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
	MessageDataList current_list;
	// Swap message list
	{
		std::lock_guard lk(message_list_mtx_);
		current_list.swap(message_list_);
	}
	for (auto const& it : current_list) {
		processMsg(it);
	}
}

void PacketParser::pushMsg(MessageData msgData)
{
	std::lock_guard lk(message_list_mtx_);
	message_list_.push_back(std::move(msgData));
}

void PacketParser::processMsg(MessageData const& msgData)
{
	auto it = cmd_message_map_.find(msgData.msg_id);
	if (it == cmd_message_map_.end()) {
		LOG_ERROR("message Id:{} not resiger CmdMessage", msgData.msg_id);
		return;
	}
	
	constexpr auto buf_size = 1024 * 64;
	static char buf[buf_size];
	if (it->second.pMessage_ && it->second.pMessage_->ParseFromArray(buf, buf_size)) {
		if (!it->second.handler_) {
			LOG_ERROR("message id:{} not register handler", msgData.msg_id);
			return;
		}
		it->second.handler_(it->second.pMessage_, it->second.pMessage_->ByteSizeLong(),msgData.session_fd);
		it->second.pMessage_->Clear();
	}
	else {
		it->second.handler_(nullptr, 0, msgData.session_fd);
	}
}

void PacketParser::RecvGgLoginReq(const void* pData, size_t len, uint32_t session_fd)
{
	auto pReq = (PKG_GG_LS_Login_REQ*)pData;
	if (!pReq) {
		LOG_ERROR("failed can not parse proto");
		return;
	}

	auto pPlayer = g_playerCtrl->fetchPlayer();
	if (!pPlayer) {
		auto pGateSession = g_gateSessionMgr->getGateSession(session_fd);
		if (pGateSession) {
			PKG_LS_GG_Login_ACK resp;
			resp.set_result(COMMErr_system_error);
			//resp.mutable_reply()->mutable_fail()->set_errcode(LSERR_Sys_Failure);
			pGateSession->send(0, ProtoId::LS_GG_Login_ACK, resp);
		}
		return;
	}

	//pPlayer->setPTID();
	FsmEvent event{};
	event.msgID = ProtoId::GG_LS_Login_REQ;
	event.session_id_ = session_fd;
	event.isGlobalEvent = false;	
	pPlayer->onEvent(event);
}

void PacketParser::OnLogoffNtf(const void* pData, size_t len, uint32_t session_fd)
{
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
	event.msgID = GG_LS_Logoff_NTF;
	event.isGlobalEvent = true;	
	pPlayer->onEvent(event);
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
		if (pack.id == CS_HeartBeat_Req) {			
			LOG_DEBUG("Session fd:{} reply GateServer PONG", session->fd());
			session->replyPing();
			continue;
		}
		if (pack.id == CS_HeartBeat_Ack) {
			LOG_DEBUG("Session fd:{} received GateServer PONG", session->fd());
			continue;
		}
		g_packetParser->pushMsg(MessageData{ pack.id, pack.fd,session->fd(),std::string_view(pack.data,pack.sz) });
	}
	return len;
}
