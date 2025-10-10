#include <sscan/scanner.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <tgbot/tgbot.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace opts = boost::program_options;

void init_logging()
{
    logging::add_console_log(
        std::cout,
        keywords::format = "[%TimeStamp%]: %Message%"
    );

    logging::add_file_log(
        keywords::file_name = "service_%N.log",                                        
        keywords::rotation_size = 256 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::hours(24)), 
        keywords::format = "[%TimeStamp%]: %Message%"                                 
    );

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info
    );

    logging::add_common_attributes();
}

int main(int argc, const char *argv[]) {
    try {
        init_logging();

        opts::options_description desc{"Options"};
        desc.add_options()
            ("config", opts::value<std::string>()->default_value("application.yml"), "Config file");

        opts::variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        Config config = Config::load(vm["config"].as<std::string>());

        // TgBot::Bot bot(config.tgbot.token);
        // bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        //     bot.getApi().sendMessage(message->chat->id, "Hi!");
        // });
        // bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        //     printf("User wrote %s\n", message->text.c_str());
        //     if (StringTools::startsWith(message->text, "/start")) {
        //         return;
        //     }
        //     bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
        // });
        // try {
        //     printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        //     TgBot::TgLongPoll longPoll(bot);
        //     while (true) {
        //         printf("Long poll started\n");
        //         longPoll.start();
        //     }
        // } catch (TgBot::TgException& e) {
        //     printf("error: %s\n", e.what());
        // }

        BondsLoader bonds_loader {config};
        PriceLoader price_loader {config};

        Scanner scanner {config, bonds_loader, price_loader};

        BOOST_LOG_TRIVIAL(info) << "Starting securities scanner";

        scanner.start();
    }
    catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
        return 1;
    }
}