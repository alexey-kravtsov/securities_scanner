#include <config.h>

#include <yaml-cpp/yaml.h>

Config Config::load(const std::string& file_name) {
    auto applicationNode = YAML::LoadFile(file_name)["application"];
    auto rankNode = applicationNode["rank"];
    auto brokerNode = applicationNode["broker"];

    RankConfig rank {
        rankNode["host"].as<std::string>(),
        rankNode["path-template"].as<std::string>(),
        rankNode["regex"].as<std::string>(),
    };

    BrokerConfig broker {
        brokerNode["host"].as<std::string>(),
        brokerNode["auth"].as<std::string>(),
        brokerNode["metadata-path"].as<std::string>(),
        brokerNode["interest-path"].as<std::string>(),
        brokerNode["coupons-path"].as<std::string>(),
        brokerNode["price-path"].as<std::string>()
    };

    return Config {rank, broker};
}