#pragma once

#include <cstdint>

enum class MsgId : uint32_t
{
    ECHO_REQ = 1,
    ECHO_RESP = 2,

    CHAT_REQ = 3,
    CHAT_RESP = 4,
};
