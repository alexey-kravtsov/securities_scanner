#include "dto.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "price_calc.h"

void read_json_value(const std::string& json, Json::Value& out_value) {
    Json::Reader reader;
    if (!reader.parse(json.c_str(), out_value)) {
        throw std::invalid_argument { "Unable to parse json" };
    }
}

std::string write_json_value(const Json::Value& value) {
    Json::FastWriter writer;
    return writer.write(value);
}

boost::uuids::uuid parse_uid(const std::string& uid) {
    boost::uuids::string_generator gen;
    return gen(uid);
}

std::string date_iso_8601(const time_point& t) {
    std::time_t epoch_seconds = std::chrono::system_clock::to_time_t(t);
    std::stringstream stream;
    stream << std::put_time( std::gmtime(&epoch_seconds), "%FT%TZ" );
    return stream.str();
}

time_point parse_iso_8601(const std::string& date_str) {
    std::chrono::system_clock::time_point date;
    std::istringstream is { date_str };
    is >> std::chrono::parse("%Y-%m-%dT%H:%M:%S", date);
    if (is.fail()) {
        throw std::invalid_argument { "Unable to parse date" + date_str };
    }

    return date;
}

template<>
std::string to_json(const BondMetadataRequest& request) {
    Json::Value root;
    root["idType"] = request.id_type;
    root["classCode"] = request.class_code;
    root["id"] = request.id;

    return write_json_value(root);
}

template<>
std::string to_json(const AccuredInterestRequest& request) {
    Json::Value root;
    root["from"] = date_iso_8601(request.from);
    root["to"] = date_iso_8601(request.to);
    root["instrumentId"] = boost::uuids::to_string(request.uid);

    return write_json_value(root);
}

template<>
std::string to_json(const CouponsRequest& request) {
    Json::Value root;
    root["from"] = date_iso_8601(request.from);
    root["to"] = date_iso_8601(request.to);
    root["instrumentId"] = boost::uuids::to_string(request.uid);

    return write_json_value(root);
}

template<>
std::string to_json(const PriceRequest& request) {
    Json::Value root;
    Json::Value uids(Json::arrayValue);
    for (unsigned int i = 0; i < request.instrument_id.size(); i++) {
        uids[i] = boost::uuids::to_string(request.instrument_id[i]);
    }
    root["instrumentId"] = uids;
    return write_json_value(root);
}

template<>
std::string to_json(const BookRequest& request) {
    Json::Value root;
    root["instrumentId"] = boost::uuids::to_string(request.uid);
    root["depth"] = std::to_string(request.depth);

    return write_json_value(root);
}

template<> 
BondMetadataResponse parse<BondMetadataResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto instrument = json["instrument"];
    
    auto nominal = instrument["nominal"];
    auto units = std::stoi(nominal["units"].asString());
    auto nano = nominal["nano"].asInt64();
    
    return BondMetadataResponse {
        .isin = instrument["isin"].asString(),
        .uid = parse_uid(instrument["uid"].asString()),
        .name = instrument["name"].asString(),
        .nominal = calc_price(units, nano),
        .buy_available = instrument["buyAvailableFlag"].asBool(),
        .sell_available = instrument["sellAvailableFlag"].asBool(),
        .floating_coupon = instrument["floatingCouponFlag"].asBool(),
        .amortization = instrument["amortizationFlag"].asBool(),
        .subordinated = instrument["subordinatedFlag"].asBool(),
        .iis = instrument["forIisFlag"].asBool(),
        .maturity_date = parse_iso_8601(instrument["maturityDate"].asString())
    };
}

template<>
AccuredInterestResponse parse<AccuredInterestResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto interests = json["accruedInterests"];
    if (interests.size() == 0) {
        return AccuredInterestResponse { .interest = 0 };
    }

    auto value = interests[0]["value"];
    auto units = std::stoi(value["units"].asString());
    auto nano = value["nano"].asInt64();

    return AccuredInterestResponse { .interest = calc_price(units, nano) };
}

template<>
CouponsResponse parse<CouponsResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto coupons = std::vector<Coupon> {};
    auto events = json["events"];
    for (auto& event : events) {
        auto coupon_date = parse_iso_8601(event["fixDate"].asString());

        auto value = event["payOneBond"];
        auto units = std::stoi(value["units"].asString());
        auto nano = value["nano"].asInt64();
        auto interest = calc_price(units, nano);

        if (interest == 0) {
            continue;
        }

        coupons.push_back(Coupon { coupon_date, interest });
    }

    return CouponsResponse { .coupons = std::move(coupons) };
}

template<>
PriceResponse parse<PriceResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto price_entries = std::vector<PriceResponseEntry> {};
    auto response_entries = json["lastPrices"];
    for (auto& response_entry : response_entries) {
        auto uid = parse_uid(response_entry["instrumentUid"].asString());
        if (!response_entry.isMember("price")) {
            continue;
        }
        auto price = response_entry["price"];
        auto units = std::stoi(price["units"].asString());
        auto nano = price["nano"].asInt64();
        price_entries.push_back(PriceResponseEntry { uid, calc_price(units, nano) });
    }

    return PriceResponse { .last_prices = std::move(price_entries) };
}

template<>
BookResponse parse<BookResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto bids = json["bids"];
    long bid = 0;
    if (bids.size() != 0) {
        auto price = bids[0]["price"];
        auto units = std::stoi(price["units"].asString());
        auto nano = price["nano"].asInt64();
        bid = calc_price(units, nano);
    }

    auto asks = json["asks"];
    long ask = 0;
    if (bids.size() != 0) {
        auto price = asks[0]["price"];
        auto units = std::stoi(price["units"].asString());
        auto nano = price["nano"].asInt64();
        ask = calc_price(units, nano);
    }

    return BookResponse { .bid_price = bid, .ask_price = ask };
}