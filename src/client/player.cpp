#include "player.h"

#include "packetParser.h"

void Player::send(uint32_t msgId, const char* data, uint16_t len)
{
	std::cout << __FUNCTION__ << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto msg = encode_packet(msgId, data, len);// encode data by user
	send(std::move(msg));
}


void Player::recv(uint32_t msgId, const char* data, uint16_t len)
{
	std::cout << __FUNCTION__ << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";

	auto pHandler = g_packetParser->findHandle(msgId);
	if (!pHandler) {
		std::cerr << "msgid: " << msgId << " not register handler\n";
		return;
	}

	pHandler(data, len, this);
}
