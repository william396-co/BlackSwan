#include "session.h"

void Session::sendPing()
{
	send(encode_packet(CS_HeartBeat_Req, "PING", sizeof("PING")));
}

void Session::sendInnerPing()
{
	send(encode_inner_packet(CS_HeartBeat_Req, "PING", sizeof("PING"), 0));
}

void Session::replyPing()
{
	send(encode_packet(CS_HeartBeat_Ack, "PONG", sizeof("PONG")));
}

void Session::replyInnerPing()
{
	send(encode_inner_packet(CS_HeartBeat_Ack, "PONG", sizeof("PONG"), 0));
}

std::atomic_uint32_t Session::id_seed_{};
