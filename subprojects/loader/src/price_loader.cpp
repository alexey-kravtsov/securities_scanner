#include <price_loader.h>
#include "dto.h"

PriceLoader::PriceLoader(const Config& a_config, http::HttpClient& a_client) : config { a_config }, client { a_client } {}

PriceMap PriceLoader::load(const std::vector<boost::uuids::uuid>& uid) {
    auto request = PriceRequest { .instrument_id = uid };
    auto response = client.post(config.broker.price_path, to_json(request));
    auto prices = parse<PriceResponse>(response);

    auto result = PriceMap();
    result.reserve(prices.last_prices.size());
    for (auto& price : prices.last_prices) {
        result[price.instrument_id] = price.price;
    }

    return result;
}