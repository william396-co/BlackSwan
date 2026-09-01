#pragma once

#include <functional>
#include <unordered_map>
#include <string_view>

#include "utils/singleton.h"

#include "networkEx/session.h"

#include "player.h"


using MessageHandler = std::function<void(const char* data, size_t len, Player* pPlayer)>;

class PacketParser : public Singleton<PacketParser> 
{
	friend class Singleton<PacketParser>;
	using HandleMap = std::unordered_map<uint32_t, MessageHandler>;

private:
	PacketParser() = default;

public:
	~PacketParser() = default;

	// 转发给客户端
	static void forward2Client(uint32_t msgId, std::string_view data_view, SessionPtr session, uint32_t fd);
	// 消息转发给服务器
	static void forward2Server(uint32_t msgId, std::string_view data_view, SessionPtr session);
	// 转发给登录服
	static void forward2Login(uint32_t msgId, std::string_view data_view, SessionPtr session);

	static size_t onRecvClientData(const char* data, size_t len, SessionPtr session);
	static size_t onRecvServerData(const char* data, size_t len, SessionPtr session);
	static size_t onRecvLoginData(const char* data, size_t len, SessionPtr session);

	void registerHandler(uint32_t msgId, MessageHandler handler);
	MessageHandler findHandle(uint32_t msgId);
private:
	HandleMap handleMap_;
};

#define g_packetParser PacketParser::InstancePtr()