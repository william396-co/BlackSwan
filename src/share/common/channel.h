#pragma once

#include <string>
#include <variant>
#include <unordered_map>
#include <type_traits>
#include <utility>

// GameServer Type
enum class ServerType
{
	GateServer = 1, // GateServer
	LoginServer = 2, // LoginServer
	DBServer = 3, // DBServer
	GameServer = 4, // GameServer
	WorldServer = 5, // WorldServer
};

enum class Mode
{	
	Listen = 1,
	Connect = 2,
};

struct ListenItem
{
	std::string local_ip{};
	uint16_t listen_port{};
};

struct ConnectItem
{
	std::string remote_ip{};
	uint16_t remote_port{};
};

using ItemInfo = std::variant < std::monostate, ConnectItem, ListenItem > ;

using ItemPair = std::pair<std::string, uint16_t>;// ip: port
struct ChannelItem
{
public:
	ItemPair getItemInfo(Mode mode)const;
private:
	Mode mode_{ Mode::Listen };
	ItemInfo item_{ std::monostate{} };
};

using ServerInfoMap = std::unordered_map<ServerType, ChannelItem>;