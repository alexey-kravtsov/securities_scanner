#include <sscan/scanner.h>
#include <iostream>
#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>

class BondYield {
    public:
        std::string isin;
        boost::uuids::uuid uid;
        std::string name;
        double price;
        double ytm;
};

constexpr int BONDS_UPDATE_INTERVAL_HRS = 24;

Scanner::Scanner(
    const Config& a_config, 
    BondsLoader& a_bonds_loader, 
    PriceLoader& a_price_loader,
    boost::asio::thread_pool& a_pool) 
    : config { a_config },
    tz { std::chrono::locate_zone(a_config.broker.timezone) },
    storage { a_bonds_loader, tz },
    price_loader { a_price_loader },
    thread_pool { a_pool },
    bonds_update_timestamp {},
    bonds_semaphore {1},
    price_semaphore {1} {}

void Scanner::start() {
    while (true) {
        process();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void Scanner::process() {
    if (!is_working_hours()) {
        return;
    }

    bool bonds_outdated = is_bonds_outdated() ;
    if (bonds_outdated && bonds_semaphore.try_acquire()) {
        boost::asio::post(thread_pool, [&]() {update_bonds();});
    }

    if (!bonds_outdated && price_semaphore.try_acquire()) {
        boost::asio::post(thread_pool, [&]() {update_price();});
    }
}

bool Scanner::is_working_hours() {
    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    auto start_of_day = std::chrono::floor<std::chrono::days>(now.get_local_time());

    std::chrono::weekday wd{start_of_day};
    if (wd == std::chrono::Saturday || wd == std::chrono::Sunday) {
        return false;
    }

    auto time_of_day = now.get_local_time() - std::chrono::zoned_time(tz, start_of_day).get_local_time();
    if (time_of_day < std::chrono::hours(10) + std::chrono::minutes(1)) { // 10:01
        return false;
    }

    if (time_of_day > std::chrono::hours(23) + std::chrono::minutes(59)) { // 23:59
        return false;
    }

    return true;
}

bool Scanner::is_bonds_outdated() {
    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::hours>(
        now.get_local_time() - bonds_update_timestamp.get_local_time()).count() >= BONDS_UPDATE_INTERVAL_HRS;
}

void Scanner::update_bonds() {
    BOOST_LOG_TRIVIAL(debug) << "Updating bonds";
    try {
        storage.load();
        update_bonds_timestamp();
    } catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << "Error updating bonds: " << ex.what();
    }

    bonds_semaphore.release();
}

void Scanner::update_bonds_timestamp() {
    std::chrono::zoned_time zt(tz, std::chrono::system_clock::now());
    auto local_day = std::chrono::floor<std::chrono::days>(zt.get_local_time());
    bonds_update_timestamp = std::chrono::zoned_time(tz, local_day + std::chrono::hours(8));
}

void Scanner::update_price() {
    BOOST_LOG_TRIVIAL(debug) << "Updating price";
    try {
        auto bond_yields = std::vector<BondYield>();
        auto bonds = storage.get_bonds();
        auto prices = price_loader.load(bonds->bonds_uids);

        BOOST_LOG_TRIVIAL(debug) << "Total prices: " << std::to_string(prices.size());
        for (auto& entry : prices) {
            auto bond_it = bonds->bonds_map.find(entry.first);
            if (bond_it == bonds->bonds_map.end()) {
                continue;
            }

            auto& bond = bond_it->second;
            if (storage.is_blacklisted(bond.uid)) {
                continue;
            }

            auto price = entry.second / 10000.0 * bond.nominal;
            auto ytm = (bond.cash_flow / (price + bond.accured_interest) - 1) * 365.0 / bond.days_to_maturity * 100;

            if (ytm >= storage.get_ytm()) {
                bond_yields.push_back(BondYield {bond.isin, bond.uid, bond.name, price / 100, ytm});
            }
        }

        if (bond_yields.size() != 0) {
            std::sort(bond_yields.begin(), bond_yields.end(), [](BondYield& a, BondYield& b) {return a.ytm > b.ytm; });

            std::chrono::zoned_time now(tz, std::chrono::system_clock::now());
            auto next_day = std::chrono::ceil<std::chrono::days>(now.get_local_time());
            auto blacklist_until = std::chrono::zoned_time(tz, next_day + std::chrono::hours(8));

            for (auto& yield : bond_yields) {
                BOOST_LOG_TRIVIAL(debug) << yield.isin << " " << yield.ytm << " " << yield.price << " " << yield.name;
                storage.blacklist_temporally(yield.uid, blacklist_until);
            }
        }
    } catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << "Error updating price: " << ex.what();
    }

    price_semaphore.release();
}