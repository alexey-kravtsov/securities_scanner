#include <string>
#include <boost/uuid/uuid.hpp>

class BondInfo {
    public:
        const std::string isin;
        const boost::uuids::uuid uid;
        const std::string name;
        const long accured_interest;
        const long nominal;
        const long cash_flow;
        const int days_to_maturity;
};