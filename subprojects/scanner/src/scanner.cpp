#include <sscan/scanner.h>
#include <iostream>
#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>

class BondYield {
    public:
        std::string isin;
        std::string name;
        double price;
        double ytm;
};

Scanner::Scanner(
    const Config& a_config, 
    BondsLoader& a_bonds_loader, 
    PriceLoader& a_price_loader,
    boost::asio::thread_pool& a_pool) 
    : config { a_config },
    storage { a_bonds_loader },
    price_loader { a_price_loader },
    thread_pool { a_pool },
    tz { std::chrono::locate_zone("Europe/Moscow") },
    bonds_update_timestamp {},
    bonds_semaphore {1},
    price_update_timestamp {},
    price_semaphore {1} {}

void Scanner::start() {
    while (true) {
        auto now = std::chrono::system_clock::now();
        auto bonds_updated_hours_ago = std::chrono::duration_cast<std::chrono::hours>(now - bonds_update_timestamp).count();
        if (bonds_updated_hours_ago >= 24 && bonds_semaphore.try_acquire()) {
            boost::asio::post(thread_pool, [&]() {update_bonds();});
        }
    }
}

void Scanner::update_bonds() {
    try {
        storage.load();
        update_bonds_timestamp();
    } catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << ex.what();
    }

    bonds_semaphore.release();
}

void Scanner::update_bonds_timestamp() {
    std::chrono::zoned_time zt(tz, std::chrono::system_clock::now());
    auto local_day = std::chrono::floor<std::chrono::days>(zt.get_local_time());
    auto t = local_day + std::chrono::hours(8);
    bonds_update_timestamp = 
}

void Scanner::update_price() {
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