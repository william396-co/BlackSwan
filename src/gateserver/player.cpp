#include "player.h"

#include "networkEx/packet.h"

#if 0
void Player::send(uint32_t msgId, const char* data, uint16_t len)
{
	//std::cout << __FUNCTION__ << " fd:" << client_session_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	if (auto s = session_) {
		auto msg = encode_packet(msgId, data, len);
		s->forward2Client(std::move(msg));
	}
}
#endif

void Player::forward2Client(uint32_t msgId, const char* data, uint16_t len)
{
	//std::cout << __FUNCTION__ << " fd:" << client_session_->fd() << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
#if 0
	send(msgId, data, len);
#else

	if (auto s = session_) {
		auto msg = encode_packet(msgId, data, len);
		s->forward2Client(std::move(msg));
	}
#endif
}

void Player::forward2Login(uint32_t msgId, const char* data, uint16_t len, uint32_t fd)
{
	if (auto s = session_) {
		std::cout << __FUNCTION__ << " fd:" << fd << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
		auto msg = encode_net_packet(msgId, data, len, fd);// encode by user
		s->forward2Login(std::move(msg));
	}
}

void Player::forward2Server(uint32_t msgId, const char* data, uint16_t len, uint32_t fd)
{
	if (auto s = session_) {
		std::cout << __FUNCTION__ << " fd:" << fd << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
		auto msg = encode_net_packet(msgId, data, len, fd);// encode by user
		s->forward2Server(std::move(msg));
	}
}
