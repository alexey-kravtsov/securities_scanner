#include <sscan/config.h>

#include <yaml-cpp/yaml.h>

Config Config::load(const std::string& file_name) {
    auto applicationNode = YAML::LoadFile(file_name)["application"];
    auto rankNode = applicationNode["rank"];
    auto brokerNode = applicationNode["broker"];

    RankConfig rank {
        .host = rankNode["host"].as<std::string>(),
        .path_template = rankNode["path-template"].as<std::string>(),
        .regex = rankNode["regex"].as<std::string>(),
    };

    BrokerConfig broker {
        .host = brokerNode["host"].as<std::string>(),
        .auth = brokerNode["auth"].as<std::string>(),
        .metadata_path = brokerNode["metadata-path"].as<std::string>(),
        .interest_path = brokerNode["interest-path"].as<std::string>(),
        .coupons_path = brokerNode["coupons-path"].as<std::string>(),
        .price_path = brokerNode["price-path"].as<std::string>(),
        .instruments_rps = brokerNode["instruments-rps"].as<int>(),
        .price_rps = brokerNode["price-rps"].as<int>(),
    };

    return Config {rank, broker};
}