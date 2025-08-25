#include "orders_loader.h"
#include "grpc.h"
#include <iostream>

void OrdersLoader::load() {
    Grpc::connect();
}