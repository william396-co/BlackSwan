#include "packet.h"


std::string encode_packet(uint32_t msgId, const char* data, uint16_t len) {

    std::string out;
    out.resize(MsgHeaderSize + len);

    auto* header = reinterpret_cast<MsgHeader*>(out.data());
    header->id = htons(static_cast<uint16_t>(msgId));
    header->sz = htons(static_cast<uint16_t>(len));
    

    if (len > 0) {
        std::memcpy(out.data() + MsgHeaderSize, data, len);
    }
    return out;
}

bool decode_packet(const char* data, uint16_t len, Packet& out) {

    if (len < MsgHeaderSize) {
        return false;
    }

    const auto* header = reinterpret_cast<const MsgHeader*>(data);
    out.id = ntohs(header->id);
    out.sz = ntohs(header->sz);

    if (len < MsgHeaderSize + out.sz) {
        return false;
    }

    out.data = data + MsgHeaderSize;
    return true;
}

std::string encode_net_packet(uint32_t msgId, const char* data, uint16_t len, uint32_t fd)
{
    std::string out;
    out.resize(NetMsgHeaderSize + len);

    auto* header = reinterpret_cast<NetMsgHeader*>(out.data());
    header->id = htons(static_cast<uint16_t>(msgId));
    header->sz = htons(static_cast<uint16_t>(len));
    header->fd = htonl(static_cast<uint32_t>(fd));

    if (len > 0) {
        std::memcpy(out.data() + NetMsgHeaderSize, data, len);
    }
    return out;
}

bool decode_net_packet(const char* data, uint16_t len, NetPacket& out)
{
    if (len < NetMsgHeaderSize) {
        return false;
    }

    const auto* header = reinterpret_cast<const NetMsgHeader*>(data);
    out.id = ntohs(header->id);
    out.sz = ntohs(header->sz);
    out.fd = ntohl(header->fd);

    if (len < NetMsgHeaderSize + out.sz) {
        return false;
    }

    out.data = data + NetMsgHeaderSize;
    return true;
}