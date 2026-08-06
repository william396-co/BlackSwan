#include "packet.h"


std::string encode_packet(uint32_t msgId, const char* data, uint32_t len) {

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

bool decode_packet(const char* data, size_t len, DecodePacket& out) {

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