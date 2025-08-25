#include "orders_loader.h"
#include "grpc.h"
#include <iostream>

using namespace loaders;

void OrdersLoader::load() {
    Grpc::connect();
}