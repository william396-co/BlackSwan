#pragma once
#include <functional>
#include <unordered_map>

#include "utils/singleton.h"
#include "common/message.h"

#include <google/protobuf/message_lite.h>

#include "player.h"
using GSMessageHandler = std::function<void(const void* data, size_t len, PlayerPtr pPlayer)>;
using GSMessage = MessageData< GSMessageHandler>;

class GSPacketHandler : public Singleton<GSPacketHandler>
{
	using GSMessageMap = std::unordered_map<uint32_t, GSMessage>;
	friend class Singleton<GSPacketHandler>;
	GSPacketHandler() = default;
public:
	~GSPacketHandler() = default;
public:
	static void onGSLoginAck(const void* data, size_t len, PlayerPtr pPlayer);
	static void onGsToClient(const void* data, size_t len, PlayerPtr pPlayer);
public:
	void init();
	void processGsMessage(uint32_t msgId, std::string_view data_view, TransID transID);
private:	
	void registerMessage(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, GSMessageHandler handler);

private:
	GSMessageMap gsMessageMap;
};

#define g_gsPacketHandler GSPacketHandler::InstancePtr()