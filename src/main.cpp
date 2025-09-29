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

        auto& broker_conf = config.broker;
        auto t_instruments_client = http::HttpClient(broker_conf.host, broker_conf.auth, broker_conf.instruments_rps);
        auto t_price_client = http::HttpClient(broker_conf.host, broker_conf.auth, broker_conf.price_rps);

        BondsLoader bonds_loader {config, sl_client, t_instruments_client};
        PriceLoader price_loader {config, t_price_client };

        Storage storage {bonds_loader};

        Scanner scanner {config, storage, price_loader};
        scanner.init();
        scanner.start();
    }
    catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}