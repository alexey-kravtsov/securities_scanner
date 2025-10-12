#ifndef SECURITIES_SCANNER_PRICE_LOADER_H
#define SECURITIES_SCANNER_PRICE_LOADER_H

#include <sscan/config.h>
#include <sscan/http.h>
#include <unordered_map>
#include <vector>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

using PriceMap = std::unordered_map<boost::uuids::uuid, long, boost::hash<boost::uuids::uuid>>;

class PriceLoader {
    public:
        PriceLoader(const Config& config);
        
        PriceLoader(const PriceLoader& other) = delete;
        PriceLoader& operator=(const PriceLoader& other) = delete;

        PriceMap load(const std::vector<boost::uuids::uuid>& uid);
    private:
        const Config& config;
        http::HttpClient client;
};

#endif // SECURITIES_SCANNER_PRICE_LOADER_H