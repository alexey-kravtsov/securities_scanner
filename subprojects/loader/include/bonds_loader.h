#ifndef SECURITIES_SCANNER_BONDS_LOADER_H
#define SECURITIES_SCANNER_BONDS_LOADER_H

#include <bond_info.h>

#include <config.h>
#include <http.h>
#include <unordered_set>
#include <regex>

class BondsLoader {
    public:
        BondsLoader(
            const Config& config, 
            http::HttpClient& sl_client, 
            http::HttpClient& t_client);

        BondsLoader(const BondsLoader& other) = delete;
        BondsLoader& operator=(const BondsLoader& other) = delete;

        std::vector<BondInfo> load();
    private:
        const Config& config;
        http::HttpClient& sl_client;
        http::HttpClient& t_client;
        const std::regex rank_regex;

        std::unordered_set<std::string> find(const int page);
        std::optional<BondInfo> load_bond(const std::string& isin);
};

#endif // SECURITIES_SCANNER_BONDS_LOADER_H