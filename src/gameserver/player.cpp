#include "player.h"

#include "packetParser.h"

void Player::send(uint32_t msgId, const char* data, uint32_t len)
{
	std::cout << __FUNCTION__ << " client_fd:" << client_fd_ << " gate_fd:" << gateSession_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto msg = encode_inner_packet(msgId, data, len, client_fd_);
	send(std::move(msg));// encode data by user
}

void Player::recv(uint32_t msgId, const char* data, uint32_t len)
{
	std::cout << __FUNCTION__ << " client_fd:" << client_fd_ << " gate_fd:" << gateSession_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";

	auto pHandler = g_packetParser->findHandle(msgId);
	if (!pHandler) {
		std::cerr << "msgid: " << msgId << " not register handler\n";
		return;
	}

	pHandler(data,len, this);
}
