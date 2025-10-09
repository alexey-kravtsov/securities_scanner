#include <sscan/scanner.h>
#include <iostream>
#include <boost/uuid/uuid_io.hpp>

class BondYield {
    public:
        std::string isin;
        std::string name;
        double price;
        double ytm;
};

Scanner::Scanner(const Config& a_config, BondsLoader& a_bonds_loader, PriceLoader& a_price_loader) 
    : config { a_config }, storage { a_bonds_loader }, price_loader { a_price_loader } {}

void Scanner::init() {
    storage.load();
}

void Scanner::start() {
    auto bonds = storage.get_bonds();
    auto prices = price_loader.load(bonds->bonds_uids);

    auto bond_yields = std::vector<BondYield>();
    std::cout << "Total prices: " << std::to_string(prices.size()) << std::endl;
    for (auto& entry : prices) {
        auto bond_it = bonds->bonds_map.find(entry.first);
        if (bond_it == bonds->bonds_map.end()) {
            continue;
        }

        auto& bond = bond_it->second;

        auto price = entry.second / 10000.0 * bond.nominal;
        auto ytm = (bond.cash_flow / (price + bond.accured_interest) - 1) * 365.0 / bond.days_to_maturity * 100;

        if (ytm >= storage.get_ytm()) {
            bond_yields.push_back(BondYield {bond.isin, bond.name, price / 100, ytm});
        }
    }

    if (bond_yields.size() == 0) {
        return;
    }

    std::sort(bond_yields.begin(), bond_yields.end(), [](BondYield& a, BondYield& b) {return a.ytm > b.ytm; });

    for (auto& yield : bond_yields) {
        std::cout << yield.isin << " " << yield.ytm << " " << yield.price << " " << yield.name << std::endl;
    }
}