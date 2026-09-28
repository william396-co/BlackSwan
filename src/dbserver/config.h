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
};

#define g_Config Config::InstancePtr()