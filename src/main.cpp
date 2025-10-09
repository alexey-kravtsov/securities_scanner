#include <sscan/scanner.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <tgbot/tgbot.h>

namespace opts = boost::program_options;

int main(int argc, const char *argv[]) {
    try {
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
        scanner.init();
        scanner.start();
    }
    catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}