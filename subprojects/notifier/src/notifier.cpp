#include <sscan/notifier.h>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include <memory>
#include <math.h>
#include <regex>

const auto PREVIEW_OPTIONS = std::make_shared<TgBot::LinkPreviewOptions>(
    TgBot::LinkPreviewOptions {
        .isDisabled = true, 
        .url = {}, 
        .preferSmallMedia = false, 
        .preferLargeMedia = false, 
        .showAboveText = false});

const auto PARSE_MODE = "MarkdownV2";
const std::regex markdonwn_specials_pattern("[_\\,\\*\\[\\]\\(\\)\\~>\\#\\+\\-=\\|\\{\\}\\.\\!]");
const std::regex ytm_pattern("\\/ytm (\\d+\\.\\d+)");
const std::regex dtm_pattern("\\/dtm (\\d+)");

const std::string format_double(const double val) {
    double int_part;
    double fract_part = modf(val , &int_part);
    int int_part_value = std::trunc(int_part);
    int fract_part_value = std::trunc(fract_part * 100);
    return std::vformat("{}\\.{}", std::make_format_args(int_part_value, fract_part_value));
}

const std::string format_date(const zoned_time& date) {
    return std::format("{:%Y\\-%m\\-%d %H\\:%M\\:%S}", std::chrono::floor<std::chrono::seconds>(date.get_local_time()));
}

const std::string get_localized_state(const WorkingState& state) {
    switch (state) {
        case WorkingState::WORKING:
            return "работаю";
        case WorkingState::IDLE:
            return "не работаю";
        case WorkingState::OVERTIME:
            return "работаю сверхурочно";
        case WorkingState::HOLIDAY:
            return "внеплановый выходной";
    }

    throw std::invalid_argument {"unknown state"};
}

const std::string get_localized_bond(const int count) {
    int digit = count % 10;
    switch (digit) {
        case 1:
            return "облигация";
        case 2:
        case 3:
        case 4:
            return "облигации";
        default:
            return "облигаций";
    };
}

const std::string sanitize_text(const std::string text) {
    return std::regex_replace(text, markdonwn_specials_pattern, "\\-");
}

Notifier::Notifier(const Config& a_config, boost::asio::thread_pool& a_thread_pool) : 
    config {a_config},
    thread_pool {a_thread_pool},
    tgbot {config.tgbot.token},
    on_stats_requested_func {} {};

void Notifier::start() {
    tgbot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) { handle_message(message); });
    boost::asio::post(thread_pool, [&]() { long_poll();});
}

void Notifier::handle_message(TgBot::Message::Ptr message) {
    try {
        if (message->text.starts_with("/stats")) {
            handle_stats_message();
        }
        if (message->text.starts_with("/ytm")) {
            handle_ytm_message(message);
        }
        if (message->text.starts_with("/dtm")) {
            handle_dtm_message(message);
        }
        if (message->text.starts_with("/overtime")) {
            on_working_state_change_func(WorkingState::OVERTIME);
        }
        if (message->text.starts_with("/holiday")) {
            on_working_state_change_func(WorkingState::HOLIDAY);
        }
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(error) << "Error handling message: " << ex.what();
    }
}

void Notifier::handle_stats_message() {
    auto stats = on_stats_requested_func();
    auto message = std::vformat(config.tgbot.stats_template, std::make_format_args(
        stats.total_bonds_loaded,
        format_date(stats.last_bonds_loaded),
        stats.total_prices_loaded,
        format_date(stats.last_prices_loaded),
        format_double(stats.min_ytm),
        stats.min_dtm,
        get_localized_state(stats.working_state)
    ));
    send_message(message);
}

void Notifier::handle_ytm_message(TgBot::Message::Ptr message) {
    std::smatch matches;
    if (!std::regex_match(message->text, matches, ytm_pattern)) {
        send_message(config.tgbot.parse_error_template);
        return;
    }

    try {
        double d = std::stod(matches[1]);
        on_target_ytm_change_func(d);
    } catch (const std::exception& e) {
        send_message(config.tgbot.parse_error_template);
    }
}

void Notifier::handle_dtm_message(TgBot::Message::Ptr message) {
    std::smatch matches;
    if (!std::regex_match(message->text, matches, dtm_pattern)) {
        send_message(config.tgbot.parse_error_template);
        return;
    }

    try {
        int d = std::stoi(matches[1]);
        on_target_dtm_change_func(d);
    } catch (const std::exception& e) {
        send_message(config.tgbot.parse_error_template);
    }
}

void Notifier::send_greeting() {
    send_message(config.tgbot.greeting_template);
}

void Notifier::send_farewell() {
    send_message(config.tgbot.farewell_template);
}

void Notifier::send_value_set() {
    send_message(config.tgbot.value_set_template);
}

void Notifier::send_overtime_success() {
    send_message(config.tgbot.overtime_success_template);
}

void Notifier::send_overtime_fail() {
    send_message(config.tgbot.overtime_fail_template);
}

void Notifier::send_holiday_success() {
    send_message(config.tgbot.holiday_success_template);
}

void Notifier::send_holiday_fail() {
    send_message(config.tgbot.holiday_fail_template);
}

void Notifier::send_working_time_error() {
    send_message(config.tgbot.working_time_error_template);
}

void Notifier::send_bonds_update_stats(const BondsUpdateStats& stats) {
    auto message = std::vformat(config.tgbot.bonds_stats_template, std::make_format_args(
        stats.total_bonds_loaded, get_localized_bond(stats.total_bonds_loaded)));
    send_message(message);
}

void Notifier::send_price_update_stats(const PriceUpdateStats& stats) {
    std::string message;
    for (auto& price : stats.new_prices) {
        auto price_message = std::vformat(config.tgbot.price_template, std::make_format_args(
            sanitize_text(price.name),
            price.isin,
            format_double(price.ytm),
            price.dtm,
            format_double(price.price)
        ));
        message += price_message;
    }
    send_message(message);
}

void Notifier::on_stats_requested(const std::function<ScannerStats ()>& func) {
    on_stats_requested_func = func;
}

void Notifier::on_target_ytm_change(const std::function<void (double)>& func) {
    on_target_ytm_change_func = func;
}

void Notifier::on_target_dtm_change(const std::function<void (int)>& func) {
    on_target_dtm_change_func = func;
}

void Notifier::on_working_state_change(const std::function<void (WorkingState)>& func) {
    on_working_state_change_func = func;
}

void Notifier::long_poll() {
    TgBot::TgLongPoll longPoll(tgbot);
    while (true) {
        try {
            longPoll.start();
        } catch (TgBot::TgException& ex) {
            BOOST_LOG_TRIVIAL(error) << "Error polling tg: " << ex.what();
        }
    }
}

void Notifier::send_message(const std::string& message) {
    tgbot.getApi().sendMessage(
        config.tgbot.chat_id,
         message,
         PREVIEW_OPTIONS,
         nullptr,
         nullptr,
         PARSE_MODE
    );
}