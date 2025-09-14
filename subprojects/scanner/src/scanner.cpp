#include "scanner.h"

Scanner::Scanner(const Config& a_config, BondsLoader& a_bonds_loader, OrdersLoader& a_orders_loader) :
    config { a_config }, bonds_loader { a_bonds_loader }, orders_loader { a_orders_loader } {}

void Scanner::init() {

}

void Scanner::start() {
    bonds_loader.load();
}