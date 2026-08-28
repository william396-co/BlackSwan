#pragma once

#include <numeric>

enum class MsgId : uint32_t
{
    ECHO_REQ = 1,
    ECHO_RESP = 2,

    CHAT_REQ = 3,
    CHAT_RESP = 4,
};

constexpr auto CMD_PING = std::numeric_limits<uint32_t>::max() - 1;// PING
constexpr auto CMD_PONG = std::numeric_limits<uint32_t>::max();    //PONG