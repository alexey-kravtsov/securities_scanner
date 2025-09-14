#ifndef CONFIG_H
#define CONFIG_H

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
};

class Config {
    public:
        RankConfig rank;
        BrokerConfig broker;

        static Config load(const std::string& path);
};

#endif // CONFIG_H