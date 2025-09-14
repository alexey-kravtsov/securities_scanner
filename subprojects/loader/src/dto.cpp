#include "dto.h"

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

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

long calc_price(const long units, const long nano) {
    return units * 100 + nano / 10000000;
}

template<>
std::string to_json(const BondMetadateRequest& request) {
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
BondMetadataResponse parse<BondMetadataResponse>(const std::string& json_str) {
    Json::Value json;
    read_json_value(json_str, json);

    auto instrument = json["instrument"];
    
    BondMetadataResponse metadata;
    metadata.isin = instrument["isin"].asString();
    metadata.uid = parse_uid(instrument["uid"].asString());
    metadata.name = instrument["name"].asString();

    auto nominal = instrument["nominal"];
    auto units = std::stoi(nominal["units"].asString());
    auto nano = nominal["nano"].asInt64();
    metadata.nominal = calc_price(units, nano);

    metadata.buy_available = instrument["buyAvailableFlag"].asBool();
    metadata.sell_available = instrument["sellAvailableFlag"].asBool();
    metadata.floating_coupon = instrument["floatingCouponFlag"].asBool();
    metadata.amortization = instrument["amortizationFlag"].asBool();
    metadata.iis = instrument["forIisFlag"].asBool();
    metadata.maturity_date = parse_iso_8601(instrument["maturityDate"].asString());
    
    return metadata;
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
        auto coupon_date = parse_iso_8601(event["couponDate"].asString());

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