#include <storage.h>

Storage::Storage(BondsLoader& bonds_loader) : loader {bonds_loader} {}

void Storage::load() {
    auto bonds_vec = loader.load();
    auto bonds_map_ptr = std::shared_ptr<BondsMap>(new BondsMap());
    bonds_map_ptr->reserve(bonds_vec.size());
    for (auto& bond : bonds_vec) {
        bonds_map_ptr->insert({bond.uid, bond});
    }
    bonds_map = bonds_map_ptr;
}

std::shared_ptr<BondsMap> Storage::get_bonds() {
    return bonds_map;
}

double Storage::get_ytm() {
    return 22.0;
}