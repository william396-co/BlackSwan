#pragma once

#include <functional>
#include <unordered_map>
#include <string_view>
#include <list>
#include <mutex>
#include <string>
#include <utility>
#include <memory>

#include <google/protobuf/message_lite.h>

#include "utils/singleton.h"
#include "networkEx/session.h"

#include "player.h"

using MessageHandler = std::function<void(const void* data, size_t len, uint32_t session_fd)>;

class MessageData
{
public:	
	MessageData(uint32_t id, uint32_t client_fd, uint32_t session_fd, std::string_view data_view)
		:msg_id{ id },
		client_fd{ client_fd },
		session_fd{ session_fd },
		tick{ xtime::now() }
	{
		data.reserve(data_view.size());
		data = data_view;
	}

	uint32_t msg_id{};
	uint32_t client_fd{};// client fd
	uint32_t session_fd{};// session fd
	std::string data;
	time_t tick;
};
using MessageDataList = std::list<MessageData>;

struct CmdMessage {
	CmdMessage() = default;
	CmdMessage(::google::protobuf::MessageLite* pMsg, MessageHandler handler)
		:pMessage_{std::move(pMsg) },
		handler_{std::move(handler)}
	{
	}
	~CmdMessage() {
		delete pMessage_;
	}
	MessageHandler handler_{};
	::google::protobuf::MessageLite* pMessage_{};
};

class PacketParser : public Singleton<PacketParser> 
{
	friend class Singleton<PacketParser>;
	using CmdMessageMap = std::unordered_map<uint32_t, CmdMessage>;

	PacketParser() = default;
public:
	~PacketParser() = default;

	// handle message from gate
	static void handleMessage(uint32_t msgId, std::string_view data_view, SessionPtr gate_session, uint32_t fd);
	// on Recv data
	static size_t onRecvData(const char* data, size_t len, SessionPtr session);

public:
	void Init();
private:
	CmdMessage findCmdMessage(uint32_t msgId);
	// Register Command
	void registerCommand(uint32_t msgId, ::google::protobuf::MessageLite* pMsg, MessageHandler handler);
public:
	void onUpdate();
	
private:
	void pushMsg(MessageData msgData);
	void processMsg(MessageData const& msgData);
private:
	static void RecvGgLoginReq(const void* pData, size_t len, uint32_t session_fd);
	static void OnLogoffNtf(const void* pData, size_t len, uint32_t session_fd);
private:
	std::mutex message_list_mtx_;
	MessageDataList message_list_;
	CmdMessageMap cmd_message_map_;
};

#define g_packetParser PacketParser::InstancePtr()