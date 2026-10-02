#pragma once

#include "utils/singleton.h"
#include "networkEx/packet.h"
#include "common/message.h"

#include <functional>
#include <utility>
#include <unordered_map>
#include <google/protobuf/message_lite.h>

#include "player.h"

using LoginMessageHandler = std::function<void(const void* data, size_t len, PlayerPtr pPlayer)>;
using LoginMessage = MessageData<LoginMessageHandler>;

class LoginPacketHandler : public Singleton<LoginPacketHandler> 
{
	using LoginMessageMap = std::unordered_map<uint32_t,LoginMessage>;
	friend class Singleton<LoginPacketHandler>;
	LoginPacketHandler() = default;
public:
	~LoginPacketHandler() = default;
private:
	static void onLsLoginAck(const void* data, size_t len, PlayerPtr pPlayer);
	static void onLsKickOffNtf(const void* data, size_t len, PlayerPtr pPlayer);
public:

	void init();
	void processLoginMessage(uint32_t msgId, std::string_view data_view,TransID transID);
private:	
	void registerMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, LoginMessageHandler handler);
private:
	LoginMessageMap loginMessageMap_;
};


#define g_loginPacketHandler LoginPacketHandler::InstancePtr()