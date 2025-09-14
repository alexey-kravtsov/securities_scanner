#include <bonds_loader.h>

#include "dto.h"
#include <iostream>
#include <unordered_set>
#include <format>
#include <boost/algorithm/string.hpp>

namespace beast = boost::beast;

const int MAX_PAGES_COUNT = 1;
const int MIN_DAYS_TO_MATURITY = 100;

BondsLoader::BondsLoader(
    const Config& a_config, 
    http::HttpClient& a_sl_client, 
    http::HttpClient& a_t_client) 
    : config {a_config}, 
    sl_client { a_sl_client },
    t_client { a_t_client },
    rank_regex {std::regex {config.rank.regex}} {}

std::vector<BondInfo> BondsLoader::load() {
    // for (int page = 1; page <= MAX_PAGES_COUNT; page++) {
    //     auto isin_set = find(sl_client, page);
    //     if (isin_set.size() == 0) {
    //         break;
    //     }

    //     std::cout << std::to_string(isin_set.size()) << std::endl;
    // }

    load_bond("123");

    sl_client.shutdown();

    return {};
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
        BondMetadateRequest request { .id = bond_isin };
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

    CouponsRequest coupons_request { .from = now, .to = metadata.maturity_date, .uid = metadata.uid };
    auto coupons_response = t_client.post(config.broker.coupons_path, to_json(coupons_request));
    auto coupons = parse<CouponsResponse>(coupons_response);
    
    long cash_flow = metadata.nominal;
    for (auto& coupon : coupons.coupons) {
        cash_flow += coupon.interest;
    }

    time_point maturity_date;
    if (coupons.coupons.size() > 0) {
        maturity_date = coupons.coupons[coupons.coupons.size() - 1].date;
    } else {
        maturity_date = metadata.maturity_date;
    }
    auto maturity_interval = maturity_date - std::chrono::system_clock::now();
    int days_to_maturity = std::chrono::duration_cast<std::chrono::days>(maturity_interval).count();

    if (!metadata.buy_available || 
        !metadata.sell_available ||
        metadata.floating_coupon ||
        metadata.amortization ||
        !metadata.iis ||
        days_to_maturity < MIN_DAYS_TO_MATURITY) {
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
            .days_to_maturity = days_to_maturity
        }
    };
}