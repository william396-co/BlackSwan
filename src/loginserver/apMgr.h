#pragma once

#include <mutex>

#include "share/utils/singleton.h"
#include "share/utils/xtime.h"

#include "authInfo.h"

enum class RESULT {
	failed = -1, sys_error = -2, time_out = -3, success = 0,
	miss_pwd, no_user, not_open,
	last_Result
};

enum class OPERATION {
	regist = 0,
	authen_ap,
	changepwd,
	last_Operation
};

class Player;
class APMsg
{
public:
	APMsg(AuthInfoPtr auth, OPERATION operation, RESULT result, int32_t error)
		:authInfo{ auth },
		tick(xtime::now()),
		operation{operation},
		result{ result },
		nErrorCode{ error }
	{
	}
	AuthInfoPtr getAuth()const { return authInfo; }
	OPERATION getOperation()const { return operation; }
	RESULT getResult()const { return result; }
	int32_t getErrorCode()const { return nErrorCode; }
	time_t getTimeStamp()const { return tick; }
private:
	AuthInfoPtr authInfo;
	time_t tick;
	OPERATION operation;
	RESULT result;
	int32_t nErrorCode;
};
using APMsgList = std::list<APMsg>;

class APMgr : public Singleton<APMgr> 
{
	friend class Singleton<APMgr>;
	APMgr() = default;
public:
	~APMgr();

	void onUpdate();
	
	void operateResult(AuthInfoPtr pAuth, OPERATION operation, RESULT result, int nErrorCode = 0);
	void checkApLoginData(Player* pPlayer);

	void onAPAuthResult(AuthInfoPtr pAuth, RESULT result, int errorCode);
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