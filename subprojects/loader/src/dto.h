#ifndef SECURITIES_SCANNER_LOADER_DTO_H
#define SECURITIES_SCANNER_LOADER_DTO_H

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>
#include <json/json.h>
#include <vector>

using time_point = std::chrono::system_clock::time_point;

struct BondMetadataRequest {
    std::string id_type = "INSTRUMENT_ID_TYPE_TICKER";
    std::string class_code = "TQCB";
    std::string id;
};

struct AccuredInterestRequest {
    time_point from;
    time_point to;
    boost::uuids::uuid uid;
};

struct CouponsRequest {
    time_point from;
    time_point to;
    boost::uuids::uuid uid;
};

struct BondMetadataResponse {
    std::string isin;
    boost::uuids::uuid uid;
    std::string name;
    long nominal;
    bool buy_available;
    bool sell_available;
    bool floating_coupon;
    bool amortization;
    bool subordinated;
    bool iis;
    time_point maturity_date;
};

struct AccuredInterestResponse {
    long interest;
};

struct PriceRequest {
    std::vector<boost::uuids::uuid> instrument_id;
};

struct PriceResponseEntry {
    boost::uuids::uuid instrument_id;
    long price;
};

struct PriceResponse {
    std::vector<PriceResponseEntry> last_prices;
};

struct Coupon {
    time_point date;
    long interest;
};

struct CouponsResponse {
    std::vector<Coupon> coupons;
};

struct BookRequest {
    boost::uuids::uuid uid;
    int depth;
};

struct BookResponse {
    long bid_price;
    long ask_price;
};

template <typename T>
std::string to_json(const T& value);

template <typename T>
T parse(const std::string& json);

#endif // SECURITIES_SCANNER_LOADER_DTO_H