#ifndef SECURITIES_SCANNER_SCANNER_H
#define SECURITIES_SCANNER_SCANNER_H

#include <sscan/config.h>
#include <sscan/bonds_loader.h>
#include <sscan/price_loader.h>
#include <boost/asio/thread_pool.hpp>
#include <semaphore>
#include <shared_mutex>
#include <chrono>

class Scanner {
    public:
        Scanner(const Config& config, BondsLoader& bonds_loader, PriceLoader& price_loader, boost::asio::thread_pool& thread_pool);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void start();
        void process();
    private:

        template<typename V>
        using UidsMap = std::unordered_map<boost::uuids::uuid, V, boost::hash<boost::uuids::uuid>>;
        using UidSet = std::vector<boost::uuids::uuid>;
        using zoned_time = std::chrono::zoned_time<std::chrono::_V2::system_clock::duration, const std::chrono::time_zone*>;

        class Bonds {
            public:
                const UidsMap<BondInfo> bonds_map;
                const UidSet bonds_uids;
        };

        class Storage {
            public:
                Storage(BondsLoader& loader, const std::chrono::time_zone* a_tz);

                Storage(const Storage& other) = delete;
                Storage& operator=(const Storage& other) = delete;

                Storage(Storage&& other) = default;
                Storage& operator=(Storage&& other) = default;

                void load();
                std::shared_ptr<Bonds> get_bonds();
                double get_ytm();

                void blacklist_temporally(const boost::uuids::uuid& uid, const zoned_time& until);
                bool is_blacklisted(const boost::uuids::uuid& uid);
            private:
                BondsLoader& loader;
                const std::chrono::time_zone* tz;
                std::shared_ptr<Bonds> bonds;

                std::shared_mutex temporally_blacklist_m;
                UidsMap<zoned_time> temporally_blacklisted_bonds;
        };

        const Config& config;
        const std::chrono::time_zone* tz;
        Storage storage;
        PriceLoader& price_loader;
        boost::asio::thread_pool& thread_pool;
        
        zoned_time bonds_update_timestamp;
        std::counting_semaphore<1> bonds_semaphore;
        std::counting_semaphore<1> price_semaphore;

        bool is_working_hours();
        bool is_bonds_outdated();
        void update_bonds();
        void update_bonds_timestamp();
        void update_price();
};

#endif // SECURITIES_SCANNER_SCANNER_H