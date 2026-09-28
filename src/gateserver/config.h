#pragma once

#include <string>

#include "share/utils/singleton.h"
#include "share/common/channel.h"

class Config : public Singleton<Config>
{
	friend class Singleton<Config>;
	Config() = default;
public:
	~Config() = default;

	bool init();
private:
	bool loadCfg();
	bool loadChannelCfg();

public:
	inline uint16_t getListenPort()const { return listen_port_; }
	inline ServerType getServerType()const { return serverType_; }
private:
	uint16_t listen_port_{};
	uint32_t listenNum_{};
	ServerInfoMap serverInfoMap_;
	ServerType serverType_{ ServerType::GateServer };
};

#define g_Config Config::InstancePtr()