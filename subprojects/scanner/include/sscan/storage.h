#ifndef SECURITIES_SCANNER_STORAGE_H
#define SECURITIES_SCANNER_STORAGE_H

#include <memory>
#include <sscan/bonds_loader.h>

using BondsMap = std::unordered_map<boost::uuids::uuid, BondInfo, boost::hash<boost::uuids::uuid>>;

class Storage {
    public:
        Storage(BondsLoader& loader);

        Storage(const Storage& other) = delete;
        Storage& operator=(const Storage& other) = delete;

        Storage(Storage&& other) = default;
        Storage& operator=(Storage&& other) = default;

        void load();
        std::shared_ptr<BondsMap> get_bonds();
        double get_ytm();
    private:
        BondsLoader& loader;
        std::shared_ptr<BondsMap> bonds_map;
};

#endif // SECURITIES_SCANNER_STORAGE_H