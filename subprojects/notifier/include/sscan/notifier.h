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

enum class WorkingState { WORKING, IDLE, OVERTIME, HOLIDAY };

struct ScannerStats {
    u_int64_t total_bonds_loaded;
    zoned_time last_bonds_loaded;

    u_int64_t total_prices_loaded;
    zoned_time last_prices_loaded;

    WorkingState working_state;

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
        void send_farewell();
        void send_value_set();
        void send_bonds_update_stats(const BondsUpdateStats& stats);
        void send_price_update_stats(const PriceUpdateStats& stats);
        void send_overtime_success();
        void send_overtime_fail();
        void send_holiday_success();
        void send_holiday_fail();
        void send_working_time_error();

        void on_stats_requested(const std::function<ScannerStats ()>& func);
        void on_target_ytm_change(const std::function<void (double)>& func);
        void on_target_dtm_change(const std::function<void (int)>& func);
        void on_working_state_change(const std::function<void (WorkingState)>& func);
    private:
        const Config& config;
        boost::asio::thread_pool& thread_pool;
        TgBot::Bot tgbot;

        std::function<ScannerStats ()> on_stats_requested_func;
        std::function<void (double)> on_target_ytm_change_func;
        std::function<void (int)> on_target_dtm_change_func;
        std::function<void (WorkingState)> on_working_state_change_func;

        void handle_message(TgBot::Message::Ptr message);
        void handle_stats_message();
        void handle_ytm_message(TgBot::Message::Ptr message);
        void handle_dtm_message(TgBot::Message::Ptr message);
        void long_poll();
        void send_message(const std::string& message);
};

#endif // SECURITIES_SCANNER_NOTIFIER_H