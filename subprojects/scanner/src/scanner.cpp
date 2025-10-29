#include <sscan/scanner.h>
#include <iostream>
#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include "storage.h"

constexpr int BONDS_UPDATE_INTERVAL_HRS = 24;

bool is_weekend(const zoned_time& now) {
    auto start_of_day = std::chrono::floor<std::chrono::days>(now.get_local_time());
    std::chrono::weekday wd{start_of_day};
    return wd == std::chrono::Saturday || wd == std::chrono::Sunday;
}

bool is_working_hours(const zoned_time& now, const std::chrono::time_zone* tz) {
    auto start_of_day = std::chrono::floor<std::chrono::days>(now.get_local_time());
    auto time_of_day = now.get_sys_time() - std::chrono::zoned_time(tz, start_of_day).get_sys_time();
    if (time_of_day < std::chrono::hours(10) + std::chrono::minutes(1)) { // 10:01
        return false;
    }

    if (time_of_day > std::chrono::hours(23) + std::chrono::minutes(59)) { // 23:59
        return false;
    }

    return true;
}

Scanner::~Scanner() = default;

Scanner::Scanner(
    const Config& a_config, 
    BondsLoader& a_bonds_loader, 
    PriceLoader& a_price_loader,
    Notifier& a_notifier,
    boost::asio::thread_pool& a_pool) 
    : config { a_config },
    tz { std::chrono::locate_zone(a_config.broker.timezone) },
    storage { new Storage(a_bonds_loader, tz) },
    price_loader { a_price_loader },
    notifier { a_notifier },
    thread_pool { a_pool },
    stats { },
    bonds_sem {1},
    price_sem {1} {}

void Scanner::start() {

    stats.working_state = WorkingState::IDLE;

    notifier.on_stats_requested([&]() { 
        return stats;
    });

    notifier.on_target_ytm_change([&](double ytm) {
        storage->set_min_ytm(ytm);
        storage->reset_blacklist();
        stats.min_ytm = ytm;
        notifier.send_value_set();
    });

    notifier.on_target_dtm_change([&](int dtm) {
        storage->set_min_dtm(dtm);
        storage->reset_blacklist();
        stats.min_dtm = dtm;
        notifier.send_value_set();
    });

    notifier.on_working_state_change([&](WorkingState state) {
        auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
        if (!is_working_hours(now, tz)) {
            notifier.send_working_time_error();
            return;
        }

        auto current_state = stats.working_state;

        if (state == WorkingState::OVERTIME) {
            if (current_state == WorkingState::WORKING || current_state == WorkingState::OVERTIME) {
                notifier.send_overtime_fail();
            } else {
                stats.working_state = state;
                notifier.send_overtime_success();
            }
        }

        if (state == WorkingState::HOLIDAY) {
            if (current_state == WorkingState::HOLIDAY) {
                notifier.send_holiday_fail();
            } else {
                stats.working_state = state;
                notifier.send_holiday_success();
            }
        }
    });

    while (true) {
        try {
            process();
        } catch (const std::exception& ex) {
            BOOST_LOG_TRIVIAL(error) << ex.what();
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void Scanner::process() {
    stats.min_ytm = storage->get_min_ytm();
    stats.min_dtm = storage->get_min_dtm();

    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    auto working_hours = is_working_hours(now, tz);

    if (!working_hours) {
        if (stats.working_state == WorkingState::WORKING || stats.working_state == WorkingState::OVERTIME) {
            try {
                notifier.send_farewell();
            } catch (const std::exception& ex) {
                BOOST_LOG_TRIVIAL(error) << ex.what();
            }
        }
        stats.working_state = WorkingState::IDLE;
        return;
    }

    if (stats.working_state == WorkingState::HOLIDAY) {
        return;
    }

    auto weekend = is_weekend(now);
    if (stats.working_state != WorkingState::OVERTIME && weekend) {
        return;
    }

    if (stats.working_state == WorkingState::IDLE) {
        try {
            notifier.send_greeting();
        } catch (const std::exception& ex) {
            BOOST_LOG_TRIVIAL(error) << ex.what();
        }
        stats.working_state = WorkingState::WORKING;
    }

    bool bonds_outdated = is_bonds_outdated() ;
    if (bonds_outdated && bonds_sem.try_acquire()) {
        boost::asio::post(thread_pool, [&]() {
            try {
                auto bonds_loaded = storage->load();
                notifier.send_bonds_update_stats(BondsUpdateStats { bonds_loaded });

                std::chrono::zoned_time zt(tz, std::chrono::system_clock::now());
                auto local_day = std::chrono::floor<std::chrono::days>(zt.get_local_time());
                stats.last_bonds_loaded = std::chrono::zoned_time(tz, local_day + std::chrono::hours(8));
                stats.total_bonds_loaded = bonds_loaded;
            } catch (const std::exception& ex) {
                BOOST_LOG_TRIVIAL(error) << "Error updating bonds: " << ex.what();
            }
            
            bonds_sem.release();
        });
    }

    if (!bonds_outdated && price_sem.try_acquire()) {
        boost::asio::post(thread_pool, [&]() {
            try {
                auto prices = update_prices();
                if (prices.new_prices.size() != 0) {
                    notifier.send_price_update_stats(prices);
                }
                temp_blacklist_bonds(prices);
                stats.last_prices_loaded = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
                stats.total_prices_loaded = prices.total_prices;
            } catch (const std::exception& ex) {
                BOOST_LOG_TRIVIAL(error) << "Error updating prices: " << ex.what();
            }

            price_sem.release();
        });
    }
}

bool Scanner::is_bonds_outdated() {
    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    return std::chrono::duration_cast<std::chrono::hours>(
        now.get_sys_time() - stats.last_bonds_loaded.get_sys_time()).count() >= BONDS_UPDATE_INTERVAL_HRS;
}

PriceUpdateStats Scanner::update_prices() {
    auto prices = PriceMap{};
    auto new_prices = std::vector<BondYield>();
    BOOST_LOG_TRIVIAL(debug) << "Updating prices";
    try {
        auto bonds = storage->get_bonds();
        auto min_ytm = stats.min_ytm;
        auto min_dtm = stats.min_dtm;

        auto uids = UidSet();
        uids.reserve(bonds->size());
        for (auto& entry : *bonds) {
            auto& bond = entry.second;
            if (bond.dtm >= min_dtm) {
                uids.push_back(entry.first);
            }
        }

        prices = price_loader.load(uids);

        BOOST_LOG_TRIVIAL(debug) << "Total prices: " << std::to_string(prices.size());
        for (auto& entry : prices) {
            auto bond_it = bonds->find(entry.first);
            if (bond_it == bonds->end()) {
                continue;
            }

            auto& bond = bond_it->second;
            auto price = entry.second / 10000.0 * bond.nominal;
            auto ytm = (bond.cash_flow / (price + bond.accured_interest) - 1) * 365.0 / bond.dtm * 100;

            if (ytm < min_ytm) {
                continue;
            }

            auto blacklisted_params = storage->get_blacklisted(bond.uid);
            if (blacklisted_params.has_value() && ytm - blacklisted_params.value().max_ytm < 1) {
                continue;
            }

            auto book_price = price_loader.load_book_price(bond.uid);
            if (book_price == 0) {
                continue;
            }

            price = book_price / 10000.0 * bond.nominal;
            ytm = (bond.cash_flow / (price + bond.accured_interest) - 1) * 365.0 / bond.dtm * 100;
            
            if (ytm < min_ytm) {
                continue;
            }

            if (blacklisted_params.has_value() && ytm - blacklisted_params.value().max_ytm < 1) {
                continue;
            }

            new_prices.push_back(BondYield {
                .isin = bond.isin,
                .uid = bond.uid,
                .name = bond.name,
                .ytm = ytm,
                .dtm = bond.dtm, 
                .price = price / 100
            });
        }

        if (new_prices.size() != 0) {
            std::sort(new_prices.begin(), new_prices.end(), [](BondYield& a, BondYield& b) {return a.ytm > b.ytm; });

            for (auto& price : new_prices) {
                BOOST_LOG_TRIVIAL(debug) << price.isin << " " 
                    << price.ytm << " " 
                    << price.price << " " 
                    << price.dtm << " " 
                    << price.name;
            }
        }
    } catch (const std::exception &ex) {
        BOOST_LOG_TRIVIAL(error) << "Error updating price: " << ex.what();
    }

    return PriceUpdateStats { prices.size(), new_prices };
}

void Scanner::temp_blacklist_bonds(const PriceUpdateStats& stats) {
    std::chrono::zoned_time now(tz, std::chrono::system_clock::now());
    auto next_day = std::chrono::ceil<std::chrono::days>(now.get_local_time());
    auto until = std::chrono::zoned_time(tz, next_day + std::chrono::hours(8));

    for (auto& bond : stats.new_prices) {
        storage->blacklist_temporally(bond.uid, BlacklistParams { .until = until, .max_ytm = bond.ytm });
    }
}