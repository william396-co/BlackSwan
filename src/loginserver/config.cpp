#include "config.h"

#include "share/log/log.h"
#include <toml.hpp>

#include "constdefs.h"

bool Config::Init() {

    if (!LoadCfg()) {
        return false;
    }
    return true;
}

bool Config::LoadCfg()
{
	constexpr auto cfg_file = "loginserver.toml";

	try {

		auto config = toml::parse_file(cfg_file);

		isLocalTest_ = config["Debug"]["IsLocalTest"].value_or(false);

		checkLoginUrl_ = config["Plat"]["CheckLoginUrl"].value_or("");
		clientVersion_ = config["Client"]["ClientVersion"].value_or(20190122);
		checkTime_ = config["Limit"]["CheckTime"].value_or(Default_Limit_Login_Times);
		checkCount_ = config["Limit"]["CheckCount"].value_or(Default_Login_Failed_Limited_Times);

		bOpenWhitePt_ = config["Server"]["openwhitept"].value_or(false);

	}
	catch (toml::parse_error const& err) {
		LOG_ERROR("exception:{}", err.description());
		return false;
	}
	return true;
}
