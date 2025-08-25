#include "grpc.h"
#include <iostream>
#include <experimental/scope>

void Grpc::connect() {
    std::experimental::scope_exit guard([&]() noexcept {
        std::cout << "scope exit" << std::endl;
    });

    std::cout << "grpc conneceted" << std::endl;
}