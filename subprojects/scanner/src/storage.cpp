#include <sscan/scanner.h>

Scanner::Storage::Storage(BondsLoader& bonds_loader) : loader {bonds_loader} {}

void Scanner::Storage::load() {
    auto bonds_vec = loader.load();
    auto bonds_map = BondsMap();
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