#include  "apMgr.h"

#include <iostream>
#include "log/log.h"
#include "player.h"
#include "playerCtrl.h"
#include "constdefs.h"

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

	auto now = xtime::now();
	if (now - msg.getTimeStamp() > Default_Packet_In_Queue_Time_Long) {
		operateResult(msg.getAuth(), msg.getOperation(), RESULT::time_out, 2);
	}

	switch (msg.getOperation()) {
	case OPERATION::regist:
		break;
	case OPERATION::authen_ap: {
		onAPAuthResult(msg.getAuth(), msg.getResult(), msg.getErrorCode());
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

void APMgr::checkApLoginData(Player* pPlayer)
{
	auto pAuth = std::make_shared<AuthInfo>();
	
	pAuth->setKV(gl_csAuthenID, pPlayer->getAuthenID());
}

void APMgr::onAPAuthResult(AuthInfoPtr pAuth, RESULT result, int errorCode)
{
	switch (result) {
	case RESULT::success:
		onAPSucc(pAuth);
		break;
	default:
		onAPError(pAuth, errorCode);
		break;
	}
}

void APMgr::onAPSucc(AuthInfoPtr pAuth)
{
	auto pPlayer = g_playerCtrl->findPlayer(strtoull(pAuth->getVal(gl_csHandle).c_str(), nullptr, 10));
	if (!pPlayer) {
		return;
	}

	PT_LS_Succ_Callback ptCallback(pAuth);
	FsmEvent event;
	event.callbackType = EFsmLsCallbackType::ELCT_AP_Succ;
	event.isLsInternalCall = true;
	event.isGlobalEvent = false;
	event.ptSuccCb = &ptCallback;
	pPlayer->onEvent(event);
}

void APMgr::onAPError(AuthInfoPtr pAuth, int error)
{
	auto pPlayer = g_playerCtrl->findPlayer(strtoull(pAuth->getVal(gl_csHandle).c_str(), nullptr, 10));
	if (!pPlayer) {
		return;
	}

	PT_LS_Error_Callback stCallback(pAuth, error);
	FsmEvent event;
	event.callbackType = EFsmLsCallbackType::ELCT_AP_Error;
	event.isLsInternalCall = true;
	event.isGlobalEvent = false;	
	event.ptErrorCb = &stCallback;
	pPlayer->onEvent(event);
}
