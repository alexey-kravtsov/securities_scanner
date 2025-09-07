#include "scanner.h"

Scanner::Scanner(const Config& config) {
    bonds_loader = std::unique_ptr<BondsLoader>(new BondsLoader(config));
    orders_loader = std::unique_ptr<OrdersLoader>(new OrdersLoader());
}

void Scanner::init() {

}

void Scanner::start() {
    bonds_loader->load();
}