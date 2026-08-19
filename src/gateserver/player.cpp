#include "player.h"

#include "networkEx/packet.h"

void Player::send(uint32_t msgId, const char* data, uint16_t len) 
{
	std::cout << __FUNCTION__ << " fd:" << client_session_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto msg = encode_packet(msgId, data, len);
	send(std::move(msg));
}

void Player::forward2Client(uint32_t msgId, const char* data, uint16_t len)
{
	std::cout << __FUNCTION__ << " fd:" << client_session_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	send(msgId, data, len);
}

void Player::forward2Server(uint32_t msgId, const char* data, uint16_t len,uint32_t fd)
{
	if (!game_connector_)return;

	std::cout << __FUNCTION__ << " fd:" << fd << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto msg = encode_net_packet(msgId, data, len, fd);
	send(std::move(msg));// encode by user
	game_connector_->send(std::move(msg));// encode by user
}
