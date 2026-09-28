#include "loginPacketHandler.h"
#include "player.h"
#include "playerMgr.h"

#include "proto/gg_ls.pb.h"
using namespace GateLoginCmd;

void LoginPacketHandler::processLoginMessage(uint32_t msgId, std::string_view data_view, TransID transID)
{	
	auto pPlayer = g_playerMgr->findPlayer(transID);
	if (!pPlayer) {
		LOG_ERROR("player not found:{}", transID);
		return;
	}

	auto it = loginMessageMap_.find(msgId);
	if (it == loginMessageMap_.end()) {
		LOG_ERROR("message:{} not register LoginMessage", msgId);
		return;
	}

	if (it->second.pMessage_ && it->second.pMessage_->ParseFromArray((const char*)data_view.data(), data_view.size())) {
		if (!it->second.handler_) {
			LOG_ERROR("message id:{} not register handler", msgId);
			return;
		}
		it->second.handler_(it->second.pMessage_, it->second.pMessage_->ByteSizeLong(), pPlayer.get());
		it->second.pMessage_->Clear();
	}
	else {
		it->second.handler_(nullptr, 0, pPlayer.get());
	}
}

void LoginPacketHandler::registerMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, LoginMessageHandler handler)
{
	loginMessageMap_[msgId] = LoginMessage{ pMsg,std::move(handler) };
}


void LoginPacketHandler::onLsLoginAck(const void* data, size_t len, PlayerPtr pPlayer)
{
	(void)len;
	auto pReq = (PKG_LS_GG_Login_ACK*)data;
	if (!pReq) {
		LOG_ERROR("parse proto failed");
		return;
	}
	FsmEvent event{};
	event.msgID = GateLoginMsgID::LS_GG_Login_ACK;
	event.transID = pPlayer->playerID();
	event.playerSession_ = pPlayer->getSession();
	event.pLsLoginAck = pReq;
	pPlayer->onEvent(event);
}

void LoginPacketHandler::onLsKickOffNtf(const void* data, size_t len, PlayerPtr pPlayer)
{
	(void)len;
	auto pReq = (PKG_LS_GG_Kickoff_NTF*)data;
	if (!pReq) {
		LOG_ERROR("parse proto failed");
		return;
	}
	FsmEvent event{};
	event.msgID = GateLoginMsgID::LS_GG_Kickoff_NTF;
	event.transID = pPlayer->playerID();
	event.playerSession_ = pPlayer->getSession();
	event.pLsKickNtf = pReq;
	pPlayer->onEvent(event);
}

void LoginPacketHandler::init()
{
	registerMessage(GateLoginMsgID::LS_GG_Login_ACK, new PKG_LS_GG_Login_ACK, &LoginPacketHandler::onLsLoginAck);
	registerMessage(GateLoginMsgID::LS_GG_Kickoff_NTF, new PKG_LS_GG_Kickoff_NTF, &LoginPacketHandler::onLsKickOffNtf);
}