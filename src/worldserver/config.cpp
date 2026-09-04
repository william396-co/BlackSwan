#include "config.h"

#include "share/log/log.h"
#include <toml.hpp>


bool Config::Init() {

    if (!LoadCfg()) {
        return false;
    }
    return true;
}

bool Config::LoadCfg()
{
	constexpr auto cfg_file = "worldserver.toml";
	return true;

	try {

		auto config = toml::parse_file(cfg_file);


	}
	catch (toml::parse_error const& err) {
		LOG_ERROR("exception:{}", err.description());
		return false;
	}
	return true;
}
