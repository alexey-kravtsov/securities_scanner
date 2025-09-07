#include <chrono>

const int MAX_PAGES_COUNT = 1;

struct BondMetadata {
    std::string isin;
    std::string uid;
    std::string name;
    bool buy_available;
    bool sell_available;
    bool floating_coupon;
    bool amortization;
    bool iis;
    std::chrono::system_clock::time_point maturity_date;
};