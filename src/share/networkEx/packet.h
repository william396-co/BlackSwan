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
    //uint32_t sn;// sequnce No
};
#pragma pack(pop)

constexpr auto MsgHeaderSize = sizeof(MsgHeader);

struct DecodePacket {

    uint16_t id;// msgId
    uint16_t sz;// data size
    //uint32_t sn;// sequnce No
    const char* data{};

    inline size_t size() { return sz + MsgHeaderSize; }
};


std::string encode_packet(uint32_t msgId, const char* data, uint32_t len);
bool decode_packet(const char* data, size_t len, DecodePacket& out);
