#ifndef SECURITIES_SCANNER_SCANNER_H
#define SECURITIES_SCANNER_SCANNER_H

#include <sscan/config.h>
#include <sscan/bonds_loader.h>
#include <sscan/price_loader.h>
#include <sscan/notifier.h>
#include <boost/asio/thread_pool.hpp>
#include <semaphore>
#include <shared_mutex>
#include <chrono>

class Scanner {
    public:
        Scanner(
            const Config& config,
            BondsLoader& bonds_loader,
            PriceLoader& price_loader,
            Notifier& notifier,
            boost::asio::thread_pool& thread_pool);

        ~Scanner();

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void start();
        void process();
    private:

        struct BlacklistParams;
        class Storage;

        using zoned_time = std::chrono::zoned_time<std::chrono::_V2::system_clock::duration, const std::chrono::time_zone*>;

        const Config& config;
        const std::chrono::time_zone* tz;
        std::unique_ptr<Storage> storage;
        PriceLoader& price_loader;
        Notifier& notifier;
        boost::asio::thread_pool& thread_pool;
        
        ScannerStats stats;
        std::counting_semaphore<1> bonds_sem;
        std::counting_semaphore<1> price_sem;

        bool is_bonds_outdated();
        PriceUpdateStats update_prices();
        void temp_blacklist_bonds(const PriceUpdateStats& stats);
};

#endif // SECURITIES_SCANNER_SCANNER_H