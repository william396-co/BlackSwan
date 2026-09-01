#include  "apMgr.h"

#include <iostream>
#include "log/log.h"

APMgr::~APMgr()
{
}

void APMgr::pushApMsg(APMsg msg)
{
	std::lock_guard lk(apg_list_mtx_);
	apg_list_.push_back(std::move(msg));
}

void APMgr::processApMsg(APMsg const& msg)
{
	if (!msg.getAuth()) {
		LOG_ERROR(" operation:{} result: {}", msg.getOperation(), msg.getResult());
		return;
	}
	/// PT返回数据存活时间
	constexpr auto Default_Packet_In_Queue_Time_Long = 1000 * 10;

	auto now = xtime::now();
	if (now - msg.getTimeStamp() > Default_Packet_In_Queue_Time_Long) {
		operateResult(msg.getAuth(), msg.getOperation(), RESULT::time_out, 2);
	}

	switch (msg.getOperation()) {
	case OPERATION::regist:
		break;
	case OPERATION::authen_ap: {
		checkLoginToken(msg);
		break;
	}
	default:
		LOG_ERROR("Unknown Aunthen operation Type:{}", msg.getOperation());
		break;
	}

	switch (msg.getResult()) {
	case RESULT::sys_error:
	case RESULT::failed:

		break;

		
	}
}

void APMgr::onUpdate() 
{
	APMsgList cur_list;
	{
		std::lock_guard lk(apg_list_mtx_);
		cur_list.swap(apg_list_);
	}
	for (auto const& msg : cur_list) {
		processApMsg(msg);
	}
}

void APMgr::operateResult(AuthInfoPtr pAuth, OPERATION operation, RESULT result, int nErrorCode)
{
	pushApMsg(APMsg{ pAuth,operation,result,nErrorCode });
}

void APMgr::checkLoginToken(APMsg const& msg)
{
	(void)msg;
}
