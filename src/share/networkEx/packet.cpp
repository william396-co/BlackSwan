#include "packet.h"


std::string encode_packet(uint32_t msgId, const char* data, uint16_t len) {

    std::string out;
    out.resize(MsgHeaderSize + len);

    auto* header = reinterpret_cast<MsgHeader*>(out.data());
    header->id = htonl(msgId);
    header->sz = htons(len);
    

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
    out.id = ntohl(header->id);
    out.sz = ntohs(header->sz);

    if (len < MsgHeaderSize + out.sz) {
        return false;
    }

    out.data = data + MsgHeaderSize;
    return true;
}

std::string encode_inner_packet(uint32_t msgId, const char* data, uint32_t len, uint32_t transID)
{
    std::string out;
    out.resize(InnerMsgHeaderSize + len);

	auto* header = reinterpret_cast<InnerMsgHeader*>(out.data());
	header->id = htonl(msgId);
	header->transID = htonl(transID);
    header->sz = htonl(len);

    if (len > 0) {
        std::memcpy(out.data() + InnerMsgHeaderSize, data, len);
    }
    return out;
}

bool decode_inner_packet(const char* data, uint32_t len, InnerPacket& out)
{
    if (len < InnerMsgHeaderSize) {
        return false;
    }

    const auto* header = reinterpret_cast<const InnerMsgHeader*>(data);
    out.id = ntohl(header->id);
    out.transID = ntohl(header->transID);
    out.sz = ntohl(header->sz);

    if (len < InnerMsgHeaderSize + out.sz) {
        return false;
    }

    out.data = data + InnerMsgHeaderSize;
    return true;
}