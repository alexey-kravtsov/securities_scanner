#include <scanner.h>
#include <iostream>
#include <boost/program_options.hpp>

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

        auto sl_client = http::HttpClient(config.rank.host);
        auto t_client = http::HttpClient(config.broker.host, config.broker.auth);

        BondsLoader bonds_loader {config, sl_client, t_client};
        OrdersLoader orders_loader {};

        Scanner scanner {config, bonds_loader, orders_loader};
        scanner.init();
        scanner.start();
    }
    catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}