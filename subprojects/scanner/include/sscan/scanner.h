#ifndef SECURITIES_SCANNER_SCANNER_H
#define SECURITIES_SCANNER_SCANNER_H

#include <sscan/config.h>
#include <memory>
#include <sscan/bonds_loader.h>
#include <sscan/price_loader.h>

class Scanner {
    public:
        Scanner(const Config& config, BondsLoader& bonds_loader, PriceLoader& price_loader);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void init();
        void start();
    
    private:

        using BondsMap = std::unordered_map<boost::uuids::uuid, BondInfo, boost::hash<boost::uuids::uuid>>;
        using UidSet = std::vector<boost::uuids::uuid>;

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
};

#endif // SECURITIES_SCANNER_SCANNER_H