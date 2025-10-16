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
const std::regex markdonw_specials_pattern("[_\\,\\*\\[\\]\\(\\)\\~>\\#\\+\\-=\\|\\{\\}\\.\\!]");

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

const std::string sanitize_text(const std::string text) {
    return std::regex_replace(text, markdonw_specials_pattern, "\\-");
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
            auto stats = on_stats_requested_func();
            auto message = std::vformat(config.tgbot.stats_template, std::make_format_args(
                stats.total_bonds_loaded,
                format_date(stats.last_bonds_loaded),
                stats.total_prices_loaded,
                format_date(stats.last_prices_loaded),
                format_double(stats.min_ytm),
                stats.min_dtm
            ));
            send_message(message);
        }
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(error) << "Error handling message: " << ex.what();
    }
}

void Notifier::send_greeting() {
    send_message(config.tgbot.greeting_template);
}

void Notifier::send_bonds_update_stats(const BondsUpdateStats& stats) {
    auto message = std::vformat(config.tgbot.bonds_stats_template, std::make_format_args(stats.total_bonds_loaded));
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
    on_stats_requested_func = [&func] () { return func(); };
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