#include "authInfo.h"

#include "share/log/log.h"

uint32_t AuthInfo::start_id = {};

AuthInfo::AuthInfo()
	:auth_id_{ start_id++ },
	APType_{},
	reserve_{},
	areaGroup_{}
{
	LOG_DEBUG("auth_id_:{}", auth_id_);
}

AuthInfo::~AuthInfo()
{
	LOG_DEBUG("auth_id_:{}", auth_id_);
}

void AuthInfo::setKV(std::string const& key, std::string const& val)
{
	kv_list_[key] = val;
}

void AuthInfo::setKV(std::string const& key, int val)
{
	kv_list_[key] = std::to_string(val);
}

std::string AuthInfo::getVal(std::string const& key)const
{
	auto it = kv_list_.find(key);
	if (it != kv_list_.end()) {
		return it->second;
	}
	return {};
}