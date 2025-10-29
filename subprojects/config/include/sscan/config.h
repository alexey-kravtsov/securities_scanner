#ifndef SECURITIES_SCANNER_CONFIG_H
#define SECURITIES_SCANNER_CONFIG_H

#include <string>

class LogConfig {
    public:
        const std::string level;
        const std::string format;
};

class RankConfig {
    public:
        const std::string host;
        const std::string path_template;
        const std::string regex;
        const int max_pages;
};

class BrokerConfig {
    public:
        const std::string host;
        const std::string auth;
        const std::string metadata_path;
        const std::string interest_path;
        const std::string coupons_path;
        const std::string price_path;
        const std::string book_price_path;
        const int instruments_rps;
        const int price_rps;
        const std::string timezone;
};

class TgBotConfig {
    public:
        const std::string token;
        const int64_t chat_id;
        const std::string greeting_template;
        const std::string bonds_stats_template;
        const std::string price_template;
        const std::string stats_template;
        const std::string farewell_template;
        const std::string value_set_template;
        const std::string parse_error_template;
        const std::string overtime_success_template;
        const std::string overtime_fail_template;
        const std::string holiday_success_template;
        const std::string holiday_fail_template;
        const std::string working_time_error_template;
};

class Config {
    public:
        LogConfig log;
        RankConfig rank;
        BrokerConfig broker;
        TgBotConfig tgbot;

        static Config load(const std::string& path);
};

#endif // SECURITIES_SCANNER_CONFIG_H