#include "clientPacketHandler.h"
#include "share/proto/cl_gg.pb.h"
using namespace ClientGateCmd;
#include "share/proto/gg_ls.pb.h"
using namespace GateLoginCmd;

#include "share/log/log.h"

#include "player.h"
#include "playerSession.h"
//#include "playerSessionMgr.h"
//#include "playerMgr.h"


void ClientPacketHandler::init()
{
	registerClientMessage(ClientGateMsgID::CLI_GG_Login_REQ, new PKG_CLI_GG_Login_REQ, &ClientPacketHandler::OnCliLoginReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_SelectRole_REQ, nullptr, &ClientPacketHandler::onCliSelRoleAndStartReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_CreateRole_REQ, nullptr, &ClientPacketHandler::onCliCreateRoleReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_DelRole_REQ, nullptr, &ClientPacketHandler::onCliDelRoleReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_UnDelRole_REQ, nullptr, &ClientPacketHandler::onCliUnDelRoleReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_ReselRole_REQ, nullptr, &ClientPacketHandler::onCliReselRoleReq);
	registerClientMessage(ClientGateMsgID::CLI_GG_Logout_NTF, nullptr, &ClientPacketHandler::onCliLogoutNtf);
	registerClientMessage(ClientGateMsgID::CLI_GG_VerifyUserName_REQ, nullptr, &ClientPacketHandler::onCliVerifyUserName);
	registerClientMessage(ClientGateMsgID::CLI_GG_SaveAttributeValue_REQ, nullptr, &ClientPacketHandler::onCliSaveAttrVal);
}

void ClientPacketHandler::OnCliLoginReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;

	auto pPlayer = pPlayerSession->getPlayer();
	if (!pPlayer) {
		LOG_ERROR("no player");
		return;
	}
	auto pReq = (PKG_CLI_GG_Login_REQ*)data;
	if (!pReq) {
		LOG_ERROR("parse proto failed");
		return;
	}
	FsmEvent event{};
	event.msgID = GateLoginCmd::GG_LS_Login_REQ;
	event.transID = pPlayer->playerID();
	event.playerSession_ = pPlayerSession;
	event.pLoginReq = pReq;
	pPlayer->onEvent(event);
}

void ClientPacketHandler::onCliSelRoleAndStartReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliCreateRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliDelRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliUnDelRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliReselRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliLogoutNtf(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	auto pPlayer = pPlayerSession->getPlayer();
	if (!pPlayer)return;

	FsmEvent event{};
	event.isGlobalEvent = true;
	event.msgID = CLI_GG_Logout_NTF;
	event.playerSession_ = pPlayerSession;
	event.globalEvtType = EFsmGlobalEvtType::EGET_LogoutNtf;
	pPlayer->onEvent(event);
}

void ClientPacketHandler::onCliVerifyUserName(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliSaveAttrVal(const void* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;
}

void ClientPacketHandler::onCliToGsMsg(const char* data, size_t len, PlayerSessionPtr pPlayerSession)
{
	(void)data;
	(void)len;
	(void)pPlayerSession;

	auto pPlayer = pPlayerSession->getPlayer();
	if (!pPlayer) {
		return;
	}

	// TODO Check Message if need
	
	// TODO Parse Real Data transform it 
	pPlayer->forward2Server(ClientGateMsgID::GS_GG_CLI_MSG, data, (uint32_t)len, pPlayerSession->transID());
}

void ClientPacketHandler::registerClientMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, ClientMessageHandler handler)
{
	clientMessageMap_[msgId] = ClientMessage{ pMsg,std::move(handler) };
}

void ClientPacketHandler::processClientMessage(uint32_t msgId, std::string_view data_view, PlayerSessionPtr pPlayerSession)
{
	auto it = clientMessageMap_.find(msgId);
	if (it == clientMessageMap_.end()) {
		LOG_ERROR("message Id:{} not resiger ClientMessage", msgId);
		return;
	}

	if (it->second.pMessage_ && it->second.pMessage_->ParseFromArray((const char*)data_view.data(), (int)data_view.size())) {
		if (!it->second.handler_) {
			LOG_ERROR("message id:{} not register handler", msgId);
			return;
		}
		it->second.handler_(it->second.pMessage_, it->second.pMessage_->ByteSizeLong(), pPlayerSession);
		it->second.pMessage_->Clear();
	}
	else {
		it->second.handler_(nullptr, 0, pPlayerSession);
	}
}