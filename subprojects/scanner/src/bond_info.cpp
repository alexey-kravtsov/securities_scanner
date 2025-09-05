#include <bond_info.h>

BondInfo::BondInfo(std::string b_isin,
            std::string b_uid,
            std::string b_name,
            long b_accured_interest,
            long b_cash_flow,
            int b_maturity_days) :
            isin {b_isin},
            uid {b_uid},
            name {b_name},
            accured_interest {b_accured_interest},
            cash_flow {b_cash_flow},
            maturity_days {b_maturity_days} {}