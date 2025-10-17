#include <sscan/config.h>

#include <yaml-cpp/yaml.h>

Config Config::load(const std::string& file_name) {
    auto applicationNode = YAML::LoadFile(file_name)["application"];

    auto logNode = applicationNode["log"];
    LogConfig log {
        .level = logNode["level"].as<std::string>(),
        .format = logNode["format"].as<std::string>()
    };

    auto rankNode = applicationNode["rank"];
    RankConfig rank {
        .host = rankNode["host"].as<std::string>(),
        .path_template = rankNode["path-template"].as<std::string>(),
        .regex = rankNode["regex"].as<std::string>(),
        .max_pages = rankNode["max-pages"].as<int>()
    };

    auto brokerNode = applicationNode["broker"];
    BrokerConfig broker {
        .host = brokerNode["host"].as<std::string>(),
        .auth = brokerNode["auth"].as<std::string>(),
        .metadata_path = brokerNode["metadata-path"].as<std::string>(),
        .interest_path = brokerNode["interest-path"].as<std::string>(),
        .coupons_path = brokerNode["coupons-path"].as<std::string>(),
        .price_path = brokerNode["price-path"].as<std::string>(),
        .book_price_path = brokerNode["book-price-path"].as<std::string>(),
        .instruments_rps = brokerNode["instruments-rps"].as<int>(),
        .price_rps = brokerNode["price-rps"].as<int>(),
        .timezone = brokerNode["timezone"].as<std::string>(),
    };

    auto tgbotNode = applicationNode["tgbot"];
    TgBotConfig tgbot {
        .token = tgbotNode["token"].as<std::string>(),
        .chat_id = tgbotNode["chat-id"].as<int64_t>(),
        .greeting_template = tgbotNode["greeting-template"].as<std::string>(),
        .bonds_stats_template = tgbotNode["bonds-stats-template"].as<std::string>(),
        .price_template = tgbotNode["price-template"].as<std::string>(),
        .stats_template = tgbotNode["stats-template"].as<std::string>(),
        .farewell_template = tgbotNode["farewell-template"].as<std::string>()
    };

    return Config {log, rank, broker, tgbot};
}