#include <bonds_loader.h>

#include <http.h>
#include <iostream>

void BondsLoader::load() {
    HttpClient client;
    client.connect("echo.free.beeceptor.com");
    client.get("/1");
    client.shutdown();

    std::cout << "bonds loaded" << std::endl;
}