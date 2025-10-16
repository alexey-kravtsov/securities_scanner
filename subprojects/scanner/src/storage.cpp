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

void Scanner::Storage::blacklist_temporally(const boost::uuids::uuid& uid, const zoned_time& until) {
    std::unique_lock<std::shared_mutex> lock(temporally_blacklist_m);
    temporally_blacklisted_bonds[uid] = until;
}

bool Scanner::Storage::is_blacklisted(const boost::uuids::uuid& uid) {
    auto now = std::chrono::zoned_time(tz, std::chrono::system_clock::now());
    {
        std::shared_lock<std::shared_mutex> rlock(temporally_blacklist_m);
        auto until_it = temporally_blacklisted_bonds.find(uid);
        if (until_it == temporally_blacklisted_bonds.end()) {
            return false;
        }

        if (until_it->second.get_local_time() > now.get_local_time()) {
            return true;
        }
    }

    std::unique_lock<std::shared_mutex> wlock(temporally_blacklist_m);
    auto until_it = temporally_blacklisted_bonds.find(uid);
    if (until_it == temporally_blacklisted_bonds.end()) {
        return false;
    }
    if (until_it->second.get_local_time() > now.get_local_time()) {
        return true;
    }

    temporally_blacklisted_bonds.erase(uid);
    return false;
}