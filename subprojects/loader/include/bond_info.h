#include <string>

class BondInfo {
    public:
        const std::string isin;
        const std::string uid;
        const std::string name;
        const long accured_interest;
        const long cash_flow;
        const int maturity_days;
};