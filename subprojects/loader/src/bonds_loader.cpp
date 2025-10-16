#include <sscan/bonds_loader.h>

#include "dto.h"
#include <iostream>
#include <unordered_set>
#include <format>
#include <boost/beast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

namespace beast = boost::beast;

BondsLoader::BondsLoader(const Config& a_config) : 
    config {a_config},
    sl_client {http::HttpClient{config.rank.host}},
    t_client {http::HttpClient{config.broker.host, config.broker.auth, config.broker.instruments_rps}},
    rank_regex {std::regex {config.rank.regex}} {};

std::vector<BondInfo> BondsLoader::load() {
    auto result = std::vector<BondInfo>();
    auto isins = std::unordered_set<std::string>();

    for (int page = 1; page <= config.rank.max_pages; page++) {
        BOOST_LOG_TRIVIAL(debug) << "Page: " << std::to_string(page);

        auto isin_set = find(page);
        sl_client.shutdown();
        if (isin_set.size() == 0) {
            break;
        }

        for (auto& isin : isin_set) {
            if (isins.contains(isin)) {
                continue;
            }

            auto bond = load_bond(isin);
            if (!bond.has_value()) {
                continue;
            }

            isins.insert(isin);
            result.push_back(bond.value());
        }
    }

    BOOST_LOG_TRIVIAL(debug) << "Total bonds loaded: " << std::to_string(result.size());

    return result;
}

std::unordered_set<std::string> BondsLoader::find(const int page) {
    std::unordered_set<std::string> isin_set;

    auto path = std::vformat(config.rank.path_template, std::make_format_args(page));
    auto response = sl_client.get(path);

    std::sregex_iterator it(response.begin(), response.end(), rank_regex);
    std::sregex_iterator end;

    if (it == end) {
        return isin_set;
    }
    
    for (; it != end; it++) {
        std::smatch match = *it;
        if (match.size() < 2) {
            continue;
        }

        isin_set.insert(boost::to_upper_copy<std::string>(match[1].str()));
    }

    return isin_set;
}

std::optional<BondInfo> BondsLoader::load_bond(const std::string& bond_isin) {
    std::string metadata_response;

    try {
        BondMetadataRequest request { .id = bond_isin };
        metadata_response = t_client.post(config.broker.metadata_path, to_json(request));
    } catch (http::not_found const& e) {
        return std::optional<BondInfo>{};
    }

    BondMetadataResponse metadata = parse<BondMetadataResponse>(metadata_response);

    if (bond_isin != metadata.isin) {
        return std::optional<BondInfo>{};
    }

    time_point now = std::chrono::system_clock::now();

    AccuredInterestRequest interest_request { .from = now, .to = now, .uid = metadata.uid };
    auto interest_response = t_client.post(config.broker.interest_path, to_json(interest_request));
    auto interest = parse<AccuredInterestResponse>(interest_response);

    time_point coupon_start_date = now + std::chrono::days(1);
    CouponsRequest coupons_request { .from = coupon_start_date, .to = metadata.maturity_date, .uid = metadata.uid };
    auto coupons_response = t_client.post(config.broker.coupons_path, to_json(coupons_request));
    auto coupons = parse<CouponsResponse>(coupons_response);

    long cash_flow = metadata.nominal;
    for (auto& coupon : coupons.coupons) {
        cash_flow += coupon.interest;
    }

    time_point maturity_date;
    if (coupons.coupons.size() > 0) {
        time_point max_date = coupons.coupons[0].date;
        for (auto& coupon : coupons.coupons) {
            if (max_date < coupon.date) {
                max_date = coupon.date;
            }
        }
        maturity_date = max_date;
    } else {
        maturity_date = metadata.maturity_date;
    }
    auto maturity_interval = maturity_date - std::chrono::system_clock::now();
    int dtm = std::chrono::duration_cast<std::chrono::days>(maturity_interval).count() + 3;

    if (!metadata.buy_available || 
        !metadata.sell_available ||
        metadata.floating_coupon ||
        metadata.amortization ||
        metadata.subordinated ||
        !metadata.iis) {
        return std::optional<BondInfo>{};
    }

    return std::optional<BondInfo> {
        BondInfo {
            .isin = std::move(metadata.isin),
            .uid = metadata.uid,
            .name = std::move(metadata.name),
            .accured_interest = interest.interest,
            .nominal = metadata.nominal,
            .cash_flow = cash_flow,
            .dtm = dtm
        }
    };
}