#include "gsPacketHandler.h"

#include "playerMgr.h"
#include "proto/cl_gg.pb.h"
using namespace ClientGateCmd;

void GSPacketHandler::onGSLoginAck(const void* data, size_t len, PlayerPtr pPlayer)
{
	(void)data;
	(void)len;
	(void)pPlayer;
}

void GSPacketHandler::onGsToClient(const void* data, size_t len, PlayerPtr pPlayer)
{
	pPlayer->forward2Client(ClientGateMsgID::GS_GG_CLI_MSG, (const char*)data, len);
	(void)data;
	(void)len;
	(void)pPlayer;
}

void GSPacketHandler::processGsMessage(uint32_t msgId, std::string_view data_view, TransID transID)
{
	auto pPlayer = g_playerMgr->findPlayer(transID);
	if (!pPlayer) {
		LOG_ERROR("player not found:{}", transID);
		return;
	}

	auto it = gsMessageMap.find(msgId);
	if (it == gsMessageMap.end()) {
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

void GSPacketHandler::registerMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, GSMessageHandler handler)
{
	gsMessageMap[msgId] = GSMessage{ pMsg,std::move(handler) };
}

void GSPacketHandler::init() 
{
	registerMessage(ClientGateMsgID::GS_GG_CLI_MSG, new PKG_CLI_TO_SERVER, &GSPacketHandler::onGsToClient);
}