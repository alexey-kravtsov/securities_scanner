#include <sscan/scanner.h>
#include <iostream>
#include <boost/uuid/uuid_io.hpp>

Scanner::Scanner(const Config& a_config, Storage& a_storage, PriceLoader& a_price_loader) 
    : config { a_config }, storage { a_storage }, price_loader { a_price_loader } {}

void Scanner::init() {
    storage.load();
}

void Scanner::start() {
    auto bonds = storage.get_bonds();
    auto uids = std::vector<boost::uuids::uuid>();
    uids.reserve(bonds->size());
    for (auto& entry : *bonds) {
        uids.push_back(entry.first);
    }

    auto prices = price_loader.load(uids);
    std::cout << "Total prices: " << std::to_string(prices.size()) << std::endl;
    for (auto& entry : prices) {
        auto bond_it = bonds->find(entry.first);
        if (bond_it == bonds->end()) {
            continue;
        }

        auto& bond = bond_it->second;

        auto price = entry.second / 10000.0 * bond.nominal;
        auto ytm = (bond.cash_flow / (price + bond.accured_interest) - 1) * 365.0 / bond.days_to_maturity * 100;

        if (ytm >= storage.get_ytm()) {
            std::cout << bond.isin << " " << ytm << " " << bond.name << std::endl;
        }
    }
}