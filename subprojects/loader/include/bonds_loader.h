#include <bond_info.h>

#include <config.h>
#include <http.h>
#include <unordered_set>
#include <regex>

class BondsLoader {
    public:
        BondsLoader(const Config& config);

        BondsLoader(const BondsLoader& other) = delete;
        BondsLoader& operator=(const BondsLoader& other) = delete;

        std::vector<BondInfo> load();
    private:
        const Config config;
        const std::regex rank_regex;

        std::unordered_set<std::string> find(http::HttpClient& sl_client, const int page);
        boost::optional<BondInfo> loadBond(http::HttpClient& t_client, const std::string& isin);
};