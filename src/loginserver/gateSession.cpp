#include "gateSession.h"
#include <numeric>

#include "log/log.h"

void GateSession::send(std::string msg)
{
	if (auto s = session_) {
		s->send(std::move(msg));
	}
}

void GateSession::send(uint32_t transId, uint32_t msgId, ::google::protobuf::MessageLite& refMsg)
{
	auto bodySize = refMsg.ByteSizeLong();
	if (bodySize > std::numeric_limits<uint32_t>::max()) {
		LOG_CRITICAL("protoData Bodysize:{} over bufferSize:{} msgID:{}", bodySize, std::numeric_limits<uint32_t>::max(), msgId);
		bodySize = 0;
	}
	send(encode_net_packet(msgId, bodySize ? refMsg.SerializeAsString().c_str() : nullptr, bodySize, 0));
}
