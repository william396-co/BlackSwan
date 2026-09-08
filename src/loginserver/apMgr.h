#pragma once

#include <mutex>

#include "share/utils/singleton.h"
#include "share/utils/xtime.h"

#include "authInfo.h"

class APMgr : public Singleton<APMgr> 
{
	friend class Singleton<APMgr>;
	APMgr() = default;
public:
	~APMgr();

	void onUpdate();
	
	void operateResult(AuthInfoPtr pAuth, OPERATION operation, RESULT result, int nErrorCode = 0);
	void checkApLoginData(Player* pPlayer);

	void onAPAuthenResult(AuthInfoPtr pAuth, RESULT result, int errorCode);
	void onAPSucc(AuthInfoPtr pAuth);
	void onAPError(AuthInfoPtr pAuth, int error);
private:
	void pushApMsg(APMsg msg);
	void processApMsg(APMsg const& msg);
private:
	APMsgList apg_list_;
	std::mutex apg_list_mtx_;
};

#define g_apmgr APMgr::InstancePtr()