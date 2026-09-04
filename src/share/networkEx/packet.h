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
    uint32_t id;// msgId
    uint16_t sz;// data size
};
#pragma pack(pop)
constexpr auto MsgHeaderSize = sizeof(MsgHeader);

#pragma pack(push,1)
struct InnerMsgHeader {

    uint32_t id;// msgId
	uint32_t transID;// Transparent ID
    uint32_t sz;// data size
};
#pragma pack(pop)
constexpr auto InnerMsgHeaderSize = sizeof(InnerMsgHeader);

// Packet Data to Client
struct Packet {

    uint32_t id;// msgId
    uint16_t sz;// data size
    const char* data{};

    inline size_t size() { return sz + MsgHeaderSize; }
};

// Packet Data Transfer between Servers
struct InnerPacket {
	uint32_t id;// msgId
    uint32_t transID;// Transparent ID
	uint32_t sz;// data size
	const char* data{};

	inline size_t size() { return sz + InnerMsgHeaderSize; }
};

// encode/decode packet function
std::string encode_packet(uint32_t msgId, const char* data, uint16_t len);
bool decode_packet(const char* data, uint16_t len, Packet& out);

// decode/encode InnerPacket function
std::string encode_inner_packet(uint32_t msgId, const char* data, uint32_t len, uint32_t transID);
bool decode_inner_packet(const char* data, uint32_t len, InnerPacket& out);

