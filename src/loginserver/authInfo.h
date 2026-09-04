#pragma once

#include <string>
#include <unordered_map>
#include <memory>

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
	inline void setState(AuthState state) { state_ = state; }
	inline AuthState getState()const { return state_; }
	void setKV(std::string const& key, std::string const& val);
	void setKV(std::string const& key, int val);
	std::string getVal(std::string const& key)const;
private:
	AuthState state_{};
	KeyValList kv_list_;
};
using AuthInfoPtr = std::shared_ptr<AuthInfo>;
