#pragma once

#include <memory>
#include <functional>
#include <string>
#include <utility>
#include "share/utils/singleton.h"
#include "common/message.h"

#include <google/protobuf/message_lite.h>

class PlayerSession;
using PlayerSessionPtr = std::shared_ptr<PlayerSession>;

using ClientMessageHandler = std::function<void(const void* data, size_t len, PlayerSessionPtr pPlayerSession)>;
using ClientMessage = MessageData<ClientMessageHandler>;

class ClientPacketHandler : public Singleton<ClientPacketHandler>
{
	using ClientMessageMap = std::unordered_map<uint32_t, ClientMessage>;
	friend class Singleton<ClientPacketHandler>;
	ClientPacketHandler() = default;
public:
	~ClientPacketHandler() = default;

	void init();

public:
	static void OnCliLoginReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliSelRoleAndStartReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliCreateRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliDelRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliUnDelRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliReselRoleReq(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliLogoutNtf(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliVerifyUserName(const void* data, size_t len, PlayerSessionPtr pPlayerSession);
	static void onCliSaveAttrVal(const void* data, size_t len, PlayerSessionPtr pPlayerSession);

	void onCliToGsMsg(const char* data, size_t len, PlayerSessionPtr pPlayerSession);
public:
	void processClientMessage(uint32_t msgId, std::string_view data_view, PlayerSessionPtr pPlayerSession);
private:	
	void registerClientMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, ClientMessageHandler handler);
private:
	ClientMessageMap clientMessageMap_;
};

#define g_ClientPacketHandler  ClientPacketHandler::InstancePtr()
