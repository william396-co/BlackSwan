#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "share/utils/xtime.h"


enum class AuthState
{
	begin = 0,
	ptAuth = 1,
	eKeyAuth,
	questReservePwd,
	end = -1,
};

class AuthInfo
{
	using KeyValList = std::unordered_map<std::string, std::string>;
public:
	AuthInfo();
	~AuthInfo();
	inline void setState(AuthState state) { state_ = state; }
	inline AuthState getState()const { return state_; }
	void setKV(std::string const& key, std::string const& val);
	void setKV(std::string const& key, int val);
	std::string getVal(std::string const& key)const;

	inline void setAPType(int apType) { APType_ = apType; }
	inline int getAPType()const { return APType_; }

	inline void setReserve(uint32_t reserve) { reserve_ = reserve; }
	inline uint32_t getReserve()const { return reserve_; }

	inline void setAreaGroup(uint32_t areaGroup) { areaGroup_ = areaGroup; }
	inline uint32_t getAreaGroup()const { return areaGroup_; }

	inline uint32_t getAuthID()const { return auth_id_; }
private:
	AuthState state_{};
	KeyValList kv_list_;

	uint32_t auth_id_;
	int APType_;
	uint32_t reserve_;
	uint32_t areaGroup_;
	static uint32_t start_id;
};
using AuthInfoPtr = std::shared_ptr<AuthInfo>;



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
		operation{ operation },
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
