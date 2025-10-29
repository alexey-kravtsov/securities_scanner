#include <sscan/scanner.h>

template<typename V>
using UidsMap = std::unordered_map<boost::uuids::uuid, V, boost::hash<boost::uuids::uuid>>;
using UidSet = std::vector<boost::uuids::uuid>;

struct Scanner::BlacklistParams {
    zoned_time until;
    double max_ytm;
};

class Scanner::Storage {
    public:
        Storage(BondsLoader& loader, const std::chrono::time_zone* a_tz);

        Storage(const Storage& other) = delete;
        Storage& operator=(const Storage& other) = delete;

        Storage(Storage&& other) = default;
        Storage& operator=(Storage&& other) = default;

        u_int64_t load();
        std::shared_ptr<UidsMap<BondInfo>> get_bonds();

        double get_min_ytm();
        void set_min_ytm(double ytm);

        int get_min_dtm();
        void set_min_dtm(int days);

        void blacklist_temporally(const boost::uuids::uuid& uid, const BlacklistParams& params);
        std::optional<BlacklistParams> get_blacklisted(const boost::uuids::uuid& uid);
        void reset_blacklist();
    private:
        BondsLoader& loader;
        const std::chrono::time_zone* tz;
        std::shared_ptr<UidsMap<BondInfo>> bonds;

        double min_ytm;
        int min_dtm;
        std::shared_mutex temporally_blacklist_m;
        UidsMap<BlacklistParams> temporally_blacklisted_bonds;
};