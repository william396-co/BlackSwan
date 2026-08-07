#pragma once

#include <functional>
#include <unordered_map>
#include <string_view>

#include "utils/singleton.h"

#include "networkEx/session.h"

#include "player.h"


using MessageHandler = std::function<void(const char* data, size_t len, PlayerPtr pPlayer)>;

class PacketParser : public Singleton<PacketParser> 
{
	friend class Singleton<PacketParser>;
	using HandleMap = std::unordered_map<uint32_t, MessageHandler>;

private:
	PacketParser() = default;

public:
	~PacketParser() = default;

	static void handleMessage(uint32_t msgId, std::string_view data_view, SessionPtr session);

	static size_t onRecvData(const char* data, size_t len, SessionPtr session);

	void registerHandler(uint32_t msgId, MessageHandler handler);
	MessageHandler findHandle(uint32_t msgId);
private:
	HandleMap handleMap_;
};

#define g_packetParser PacketParser::InstancePtr()