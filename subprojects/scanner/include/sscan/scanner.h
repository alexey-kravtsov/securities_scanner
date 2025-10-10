#ifndef SECURITIES_SCANNER_SCANNER_H
#define SECURITIES_SCANNER_SCANNER_H

#include <sscan/config.h>
#include <sscan/bonds_loader.h>
#include <sscan/price_loader.h>
#include <boost/asio/thread_pool.hpp>
#include <semaphore>

class Scanner {
    public:
        Scanner(const Config& config, BondsLoader& bonds_loader, PriceLoader& price_loader, boost::asio::thread_pool& thread_pool);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void start();
    private:

        using BondsMap = std::unordered_map<boost::uuids::uuid, BondInfo, boost::hash<boost::uuids::uuid>>;
        using UidSet = std::vector<boost::uuids::uuid>;
        using time_point = std::chrono::time_point<std::chrono::local_t, std::chrono::hours>;

        class Bonds {
            public:
                const BondsMap bonds_map;
                const UidSet bonds_uids;
        };

        class Storage {
            public:
                Storage(BondsLoader& loader);

                Storage(const Storage& other) = delete;
                Storage& operator=(const Storage& other) = delete;

                Storage(Storage&& other) = default;
                Storage& operator=(Storage&& other) = default;

                void load();
                std::shared_ptr<Bonds> get_bonds();
                double get_ytm();
            private:
                BondsLoader& loader;
                std::shared_ptr<Bonds> bonds;
        };

        const Config& config;
        Storage storage;
        PriceLoader& price_loader;
        boost::asio::thread_pool& thread_pool;
        
        const std::chrono::time_zone* tz;
        time_point bonds_update_timestamp;
        std::counting_semaphore<1> bonds_semaphore;

        time_point price_update_timestamp;
        std::counting_semaphore<1> price_semaphore;

        void update_bonds();
        void update_bonds_timestamp();
        void update_price();
};

#endif // SECURITIES_SCANNER_SCANNER_H