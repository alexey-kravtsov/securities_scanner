#include <bonds_loader.h>

#include "bonds_loader_utils.h"
#include <json/value.h>
#include <json/reader.h>
#include <iostream>
#include <unordered_set>
#include <format>
#include <boost/algorithm/string.hpp>

namespace beast = boost::beast;

BondsLoader::BondsLoader(const Config& a_config) 
: config {a_config}, 
  rank_regex {std::regex {config.rank.regex}} {}

std::vector<BondInfo> BondsLoader::load() {
    auto sl_client = http::HttpClient(config.rank.host);
    auto t_client = http::HttpClient(config.broker.host, config.broker.auth);

    for (int page = 1; page <= MAX_PAGES_COUNT; page++) {
        auto isin_set = find(sl_client, page);
        if (isin_set.size() == 0) {
            break;
        }

        std::cout << std::to_string(isin_set.size()) << std::endl;
    }

    loadBond(t_client, "123");

    sl_client.shutdown();
    t_client.shutdown();

    return {};
}

std::unordered_set<std::string> BondsLoader::find(http::HttpClient& sl_client,const int page) {
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

boost::optional<BondInfo> BondsLoader::loadBond(http::HttpClient& t_client, const std::string& bond_isin) {
    std::string metadata_response;

    try {
        auto metadata_request = std::vformat(config.broker.metadata_request_template, std::make_format_args(bond_isin));
        metadata_response = t_client.post(config.broker.metadata_path, metadata_request);
    } catch (http::not_found const& e) {
        return boost::optional<BondInfo>{};
    }

    Json::Reader reader;
    Json::Value metadata_json;
    if (!reader.parse(metadata_response.c_str(), metadata_json)) {
        throw std::invalid_argument { "Unable to parse metadata response" };
    }

    auto instrument = metadata_json["instrument"];
    
    auto isin = instrument["isin"].asString();

    if (bond_isin != isin) {
        return boost::optional<BondInfo>{};;
    }
    
    BondMetadata metadata;
    metadata.isin = isin;
    metadata.uid = instrument["uid"].asString();
    metadata.name = instrument["name"].asString();
    metadata.buy_available = instrument["buyAvailableFlag"].asBool();
    metadata.sell_available = instrument["sellAvailableFlag"].asBool();
    metadata.floating_coupon = instrument["floatingCouponFlag"].asBool();
    metadata.amortization = instrument["amortizationFlag"].asBool();
    metadata.iis = instrument["forIisFlag"].asBool();

    auto maturity_date_str = instrument["maturityDate"].asString();
    std::chrono::system_clock::time_point maturity_date;
    std::istringstream is { maturity_date_str };
    is >> std::chrono::parse("%Y-%m-%dT%H:%M:%S", maturity_date);
    if (is.fail()) {
        throw std::invalid_argument { "Unable to parse date" + maturity_date_str };
    }
    metadata.maturity_date = maturity_date;
    
    std::cout << metadata.uid << std::endl;
    std::cout << metadata.maturity_date << std::endl;
    
    return {};
}