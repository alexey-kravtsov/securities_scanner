#ifndef SECURITIES_SCANNER_CONFIG_H
#define SECURITIES_SCANNER_CONFIG_H

#include <string>

class RankConfig {
    public:
        const std::string host;
        const std::string path_template;
        const std::string regex;
};

class BrokerConfig {
    public:
        const std::string host;
        const std::string auth;
        const std::string metadata_path;
        const std::string interest_path;
        const std::string coupons_path;
        const std::string price_path;
        const int instruments_rps;
        const int price_rps;
};

class TgBotConfig {
    public:
        const std::string token;
};

class Config {
    public:
        RankConfig rank;
        BrokerConfig broker;
        TgBotConfig tgbot;

        static Config load(const std::string& path);
};

#endif // SECURITIES_SCANNER_CONFIG_H