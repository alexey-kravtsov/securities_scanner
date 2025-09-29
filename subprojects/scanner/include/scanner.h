#ifndef SECURITIES_SCANNER_SCANNER_H
#define SECURITIES_SCANNER_SCANNER_H

#include <config.h>
#include <memory>
#include <bonds_loader.h>
#include <price_loader.h>
#include <storage.h>

class Scanner {
    public:
        Scanner(const Config& config, Storage& storage, PriceLoader& price_loader);

        Scanner(const Scanner& other) = delete;
        Scanner& operator=(const Scanner& other) = delete;

        void init();
        void start();
    
    private:       
        const Config& config;
        Storage& storage;
        PriceLoader& price_loader;
};

#endif // SECURITIES_SCANNER_SCANNER_H