#pragma once

#include <functional>
#include <unordered_map>
#include <string_view>

#include "utils/singleton.h"
#include "networkEx/session.h"


class PacketParser : public Singleton<PacketParser> 
{
	friend class Singleton<PacketParser>;
private:
	PacketParser() = default;

public:
	~PacketParser() = default;

public:
	// Disptach Client Packet
	static void onRecvClientPacket(uint32_t msgId, std::string_view data_view, SessionPtr session);
	// Disptach Login Packet
	static void onRecvLoginPacket(uint32_t msgId, std::string_view data_view, SessionPtr session, TransID transID);
	// Disptch GameServer Packet
	static void onRecvServerPacket(uint32_t msgId, std::string_view data_view, SessionPtr session, TransID transID);


	// forward to client
	static void forward2Client(uint32_t msgId, std::string_view data_view, SessionPtr session, uint32_t transID);
	// forward to gameserver
	static void forward2Server(uint32_t msgId, std::string_view data_view, SessionPtr session);
	// forward to loginServer
	static void forward2Login(uint32_t msgId, std::string_view data_view, SessionPtr session);

	static size_t onRecvClientData(const char* data, size_t len, SessionPtr session);
	static size_t onRecvServerData(const char* data, size_t len, SessionPtr session);

	static size_t onRecvLoginData(const char* data, size_t len, SessionPtr session);
};

#define g_packetParser PacketParser::InstancePtr()