#include <sscan/scanner.h>

Scanner::Storage::Storage(BondsLoader& bonds_loader, const std::chrono::time_zone* a_tz) : 
    loader {bonds_loader},
     tz {a_tz},
     min_ytm {21.5},
     min_dtm {60},
     temporally_blacklist_m {},
     temporally_blacklisted_bonds {} {}

u_int64_t Scanner::Storage::load() {
    auto bonds_vec = loader.load();
    auto bonds_map = UidsMap<BondInfo>();
    bonds_map.reserve(bonds_vec.size());
    for (auto& bond : bonds_vec) {
        bonds_map.insert({bond.uid, bond});
    }
    
    bonds = std::make_shared<UidsMap<BondInfo>>(std::move(bonds_map));
    return bonds->size();
}

std::shared_ptr<Scanner::UidsMap<BondInfo>> Scanner::Storage::get_bonds() {
    return bonds;
}

double Scanner::Storage::get_min_ytm() {
    return min_ytm;
};

void Scanner::Storage::set_min_ytm(double ytm) {
    this->min_ytm = ytm;
}

int Scanner::Storage::get_min_dtm() {
    return min_dtm;
}

void Scanner::Storage::set_min_dtm(int days) {
    this->min_dtm = days;
}

void Scanner::Storage::blacklist_temporally(const boost::uuids::uuid& uid, const BlacklistParams& params) {
    std::unique_lock<std::shared_mutex> wlock(temporally_blacklist_m);
    temporally_blacklisted_bonds[uid] = BlacklistParams { params };
}

std::optional<Scanner::BlacklistParams> Scanner::Storage::get_blacklisted(const boost::uuids::uuid& uid) {
    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    {
        std::shared_lock<std::shared_mutex> rlock(temporally_blacklist_m);
        auto params_it = temporally_blacklisted_bonds.find(uid);
        if (params_it == temporally_blacklisted_bonds.end()) {
            return {};
        }
    }

    std::unique_lock<std::shared_mutex> wlock(temporally_blacklist_m);
    auto params_it = temporally_blacklisted_bonds.find(uid);
    if (params_it == temporally_blacklisted_bonds.end()) {
        return {};
    }

    zoned_time until = params_it->second.until;
    if (until.get_local_time() > now.get_local_time()) {
        return std::optional { params_it->second };
    }

    temporally_blacklisted_bonds.erase(uid);
    return {};
}