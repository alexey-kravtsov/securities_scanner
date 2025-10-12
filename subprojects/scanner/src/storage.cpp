#include <sscan/scanner.h>

Scanner::Storage::Storage(BondsLoader& bonds_loader, const std::chrono::time_zone* a_tz) : 
    loader {bonds_loader}, tz {a_tz}, temporally_blacklist_m {}, temporally_blacklisted_bonds {} {}

void Scanner::Storage::load() {
    auto bonds_vec = loader.load();
    auto bonds_map = UidsMap<BondInfo>();
    bonds_map.reserve(bonds_vec.size());
    for (auto& bond : bonds_vec) {
        bonds_map.insert({bond.uid, bond});
    }
    
    auto uids = UidSet();
    uids.reserve(bonds_map.size());
    for (auto& entry : bonds_map) {
        uids.push_back(entry.first);
    }

    bonds = std::make_shared<Bonds>(Bonds{ .bonds_map = std::move(bonds_map), .bonds_uids = std::move(uids)});
}

std::shared_ptr<Scanner::Bonds> Scanner::Storage::get_bonds() {
    return bonds;
}

double Scanner::Storage::get_ytm() {
    return 22.0;
};

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