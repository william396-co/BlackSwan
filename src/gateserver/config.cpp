#include "config.h"

#include "share/log/log.h"
#include <toml.hpp>

#include <string>


bool Config::init() 
{
    if (!loadChannelCfg()) {
        return false;
    }
    if (!loadCfg()) {
        return false;
    }
    return true;
}

bool Config::loadCfg()
{
	constexpr auto cfg_file = "gateserver.toml";

	try {
		auto config = toml::parse_file(cfg_file);

        listen_port_ = config["GameGate"]["ListenPort"].value_or(0);
		serverType_ = (ServerType)config["GameGate"]["ServerType"].value_or(ServerType::GateServer);

	}
	catch (toml::parse_error const& err) {
		LOG_CRITICAL("exception:{}", err.description());
		return false;
	}
	return true;
}

bool Config::loadChannelCfg()
{
#if 0
    constexpr auto cfg_file = "gatechannel.toml";

    try {

        auto config = toml::parse_file(cfg_file);

        // 读取 server id
        std::string server_id = config["connections"]["server"]["id"].value_or("");
               

        // 遍历所有 connection
        auto connections_arr = config["connections"]["connection"].as_array();
        if (!connections_arr)return false;

        for (auto const& conn_node : *connections_arr) {

            auto* conn = conn_node.as_table();
            if (!conn)continue;

            std::string mode = (*conn)["mode"].value_or("");

            if (mode == "connect") {
                auto item_arr = (*conn)["item"].as_array();
                if (!item_arr)continue;

                for (auto const& item_node : *item_arr) {

                    auto* item = item_node.as_table();
                    if (!item)continue;

                    std::string id = (*item)["id"].value_or("");
                    std::string remoteIp = (*item)["remoteip"].value_or("");
                    auto port = (uint16_t)(*item)["remoteport"].value_or(0);
                    int64_t recvBuf = (*item)["recvbuf"].value_or(0);
                    int64_t sndBuf = (*item)["sendbuf"].value_or(0);
                }
            }
            else if (mode == "listen") {

                auto item_arr = (*conn)["item"].as_array();
                if (!item_arr)continue;

                for (auto const& item_node : *item_arr) {

                    auto* item = item_node.as_table();
                    if (!item)continue;

                    std::string localIp = (*item)["localip"].value_or("");
                    auto localPort = (uint16_t)(*item)["localport"].value_or(0);
                    int64_t recvBuf = (*item)["recvbuf"].value_or(0);
                    int64_t sndBuf = (*item)["sendbuf"].value_or(0);
                }
            }
        }

    }
    catch (toml::parse_error const& err) {
        LOG_CRITICAL("exception:{}", err.description());
        return false;
    }
#endif
    return true;
}