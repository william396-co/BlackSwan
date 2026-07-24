#include "packet.h"


std::string encode_packet(uint32_t msgId, const char* data, uint32_t len) {

    std::string out;
    out.resize(MsgHeaderSize + len);

    auto* header = reinterpret_cast<MsgHeader*>(out.data());
    header->id = htonl(msgId);
    header->sz = htonl(len);
    

    if (size > 0) {
        std::memcpy(out.data() + MsgHeaderSize, data, len);
    }
    return out;
}

bool decode_packet(const char* data, size_t len, DecodePacket& out) {

    if (len < MsgHeaderSize) {
        return false;
    }

    const auto* header = reinterpret_cast<const MsgHeader*>(data);
    out.id = ntohl(header->id);
    out.sz = ntohll(header->sz);    

    if (len < MsgHeaderSize + out.size) {
        return false;
    }

    out.data = data + MsgHeaderSize;
    return true;
}