#include <string>
#include <chrono>

class BondInfo {
    public:
        const std::string isin;
        const std::string uid;
        const std::string name;
        const long accured_interest;
        const long cash_flow;
        const int maturity_days;

        BondInfo(
            std::string isin,
            std::string uid,
            std::string name,
            long accured_interest,
            long cash_flow,
            int maturity_days);
};