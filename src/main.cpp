#include <sscan/scanner.h>
#include <sscan/notifier.h>
#include <iostream>
#include <boost/program_options.hpp>

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

void init_logging(const LogConfig& config) {
    logging::add_console_log(
        std::cout,
        keywords::format = config.format
    );

    logging::add_file_log(
        keywords::file_name = "service_%N.log",                                        
        keywords::rotation_size = 256 * 1024,
        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::hours(24)), 
        keywords::format = config.format                                
    );

    boost::log::trivial::severity_level severity;
    std::istringstream{config.level} >> severity;
    logging::core::get()->set_filter(
        logging::trivial::severity >= severity
    );

    logging::add_common_attributes();
}

int main(int argc, const char *argv[]) {
    try {
        opts::options_description desc{"Options"};
        desc.add_options()
            ("config", opts::value<std::string>()->default_value("application.yml"), "Config file");

        opts::variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        Config config = Config::load(vm["config"].as<std::string>());

        init_logging(config.log);

        BondsLoader bonds_loader {config};
        PriceLoader price_loader {config};

        boost::asio::thread_pool thread_pool(16);

        Notifier notifier {config, thread_pool};
        notifier.start();

        Scanner scanner {config, bonds_loader, price_loader, notifier, thread_pool};

        BOOST_LOG_TRIVIAL(info) << "Starting securities scanner";

        scanner.start();
    }
    catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
        return 1;
    }
}