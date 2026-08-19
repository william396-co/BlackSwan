#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#if defined( WIN32 ) || defined( _WIN32 ) || defined( WIN64 ) || defined( _WIN64 )
#include <windows.h>
#elif !defined( __unix )
#define __unix
#endif

#ifdef __unix
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#pragma pack(push, 1)
struct MsgHeader 
{
    uint16_t id;// msgId
    uint16_t sz;// data size
};
#pragma pack(pop)
constexpr auto MsgHeaderSize = sizeof(MsgHeader);

#pragma pack(push,1)
struct NetMsgHeader {

    uint16_t id;// msgId
    uint16_t sz;// data size
	uint32_t fd;// origin client id
};
#pragma pack(pop)
constexpr auto NetMsgHeaderSize = sizeof(NetMsgHeader);

// Packet Data to Client
struct Packet {

    uint16_t id;// msgId
    uint16_t sz;// data size
    const char* data{};

    inline size_t size() { return sz + MsgHeaderSize; }
};

// Packet Data Transfer between Servers
struct NetPacket {
	uint16_t id;// msgId
	uint16_t sz;// data size
	uint32_t fd;// origin client fd
	const char* data{};

	inline size_t size() { return sz + NetMsgHeaderSize; }
};

// encode/decode packet function
std::string encode_packet(uint32_t msgId, const char* data, uint16_t len);
bool decode_packet(const char* data, uint16_t len, Packet& out);

// decode/encode netPacket function
std::string encode_net_packet(uint32_t msgId, const char* data, uint16_t len, uint32_t fd);
bool decode_net_packet(const char* data, uint16_t len, NetPacket& out);

