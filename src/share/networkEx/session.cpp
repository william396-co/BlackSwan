#include "session.h"

void Session::sendPing()
{
	send(encode_packet(CS_HeartBeat_Req, "PING", sizeof("PING")));
}

void Session::sendPing(uint32_t fd)
{
	send(encode_net_packet(CS_HeartBeat_Req, "PING", sizeof("PING"), fd));
}

void Session::replyPing()
{
	send(encode_packet(CS_HeartBeat_Ack, "PONG", sizeof("PONG")));
}

void Session::replyPing(uint32_t fd)
{
	send(encode_net_packet(CS_HeartBeat_Ack, "PONG", sizeof("PONG"), fd));
}

std::atomic_uint32_t Session::id_seed_{};
