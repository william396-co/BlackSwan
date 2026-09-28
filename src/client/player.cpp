#include "player.h"


#include <numeric>

#include "packetParser.h"
#include "log/log.h"

#include "proto/cl_gg.pb.h"
using namespace ClientGateCmd;

enum class ClientType
{
	PC = 0,
	IOS = 1,
	Andriod = 2,
};


void Player::send(uint32_t msgId, ::google::protobuf::MessageLite& refMsg) 
{
	auto strMsg = refMsg.SerializeAsString();
	if (strMsg.size() > std::numeric_limits<uint16_t>::max()) {
		LOG_ERROR("msgId:{} over size limit:{}", msgId, strMsg.size());
		return;
	}
	auto msg = encode_packet(msgId, strMsg.c_str(), (uint16_t)strMsg.size());
	send(std::move(msg));
}

void Player::send(uint32_t msgId, const char* data, uint16_t len)
{
	std::cout << __FUNCTION__ << " msgId:" << msgId << " data: [" << data << "]  len:" << len << "\n";
	auto msg = encode_packet(msgId, data, len);// encode data by user
	send(std::move(msg));
}

void Player::replyPing()
{
	if (connector_) {
		if (auto session = connector_->session()) {
			session->replyPing();
		}
	}
}

void Player::sendLoginReq()
{
	PKG_CLI_GG_Login_REQ req;
	req.set_clienttype((uint32_t)ClientType::PC);
	req.set_clientversion(1);
	req.set_authact("jack");
	req.set_aptype(1);
	req.set_authstr("qwe123");
	req.set_reserve(123456);
	req.set_invitecode(std::string("god"));
	send(ClientGateMsgID::CLI_GG_Login_REQ, req);
}

void Player::sendAttack(uint32_t skillId)
{
	(void)skillId;
}

void Player::sendMove(uint16_t x, uint16_t y)
{
	(void)x;
	(void)y;
}

void Player::fillData()
{
}

