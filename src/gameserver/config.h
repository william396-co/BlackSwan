#pragma once

#include <string>

#include "share/utils/singleton.h"

class Config : public Singleton<Config>
{
	friend class Singleton<Config>;
	Config() = default;
public:
	~Config() = default;

	bool Init();
private:
	bool LoadCfg();

public:
	bool IsLocalTest()const { return isLocalTest_; }
	bool bOpenWhitePt()const { return bOpenWhitePt_; }
	std::string const& getLoginUrl()const { return checkLoginUrl_; }
	int32_t getClientVersion()const { return clientVersion_; }
	int32_t getCheckTime()const { return checkTime_; }
	int32_t getCheckCount()const { return checkCount_; }
private:
	bool isLocalTest_{};
	std::string checkLoginUrl_{};
	int32_t clientVersion_{};
	int32_t checkTime_{};
	int32_t checkCount_{};
	bool bOpenWhitePt_{};
};

#define g_Config Config::InstancePtr()