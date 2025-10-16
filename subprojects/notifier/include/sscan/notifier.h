#ifndef SECURITIES_SCANNER_NOTIFIER_H
#define SECURITIES_SCANNER_NOTIFIER_H

#include <sscan/config.h>
#include <boost/uuid/uuid.hpp>
#include <boost/asio/thread_pool.hpp>
#include <tgbot/tgbot.h>
#include <vector>
#include <chrono>

using zoned_time = std::chrono::zoned_time<std::chrono::_V2::system_clock::duration, const std::chrono::time_zone*>;

struct BondsUpdateStats {
    u_int64_t total_bonds_loaded;
};

struct BondYield {
    std::string isin;
    boost::uuids::uuid uid;
    std::string name;
    double ytm;
    int dtm;
    double price;
};

struct PriceUpdateStats {
    u_int64_t total_prices;
    std::vector<BondYield> new_prices;
};

struct ScannerStats {
    u_int64_t total_bonds_loaded;
    zoned_time last_bonds_loaded;

    u_int64_t total_prices_loaded;
    zoned_time last_prices_loaded;

    double min_ytm;
    int min_dtm;
};

class Notifier {
    public:
        Notifier(const Config& config, boost::asio::thread_pool& thread_pool);

        Notifier(const Notifier& other) = delete;
        Notifier& operator=(const Notifier& other) = delete;

        void start();

        void send_greeting();
        void send_bonds_update_stats(const BondsUpdateStats& stats);
        void send_price_update_stats(const PriceUpdateStats& stats);
        void on_stats_requested(const std::function<ScannerStats ()>& func);
    private:
        const Config& config;
        boost::asio::thread_pool& thread_pool;
        TgBot::Bot tgbot;

        std::function<ScannerStats ()> on_stats_requested_func;

        void handle_message(TgBot::Message::Ptr message);
        void long_poll();
        void send_message(const std::string& message);
};

#endif // SECURITIES_SCANNER_NOTIFIER_H