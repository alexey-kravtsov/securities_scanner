#include <sscan/price_loader.h>
#include "dto.h"

PriceLoader::PriceLoader(const Config& a_config) 
    : config { a_config },
     client { http::HttpClient{config.broker.host, config.broker.auth, config.broker.price_rps}} {}

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

long PriceLoader::load_book_price(const boost::uuids::uuid& uid) {
    auto request = BookRequest { .uid = uid, .depth = 1 };
    auto response = client.post(config.broker.book_price_path, to_json(request));
    auto book_price = parse<BookResponse>(response);
    return book_price.ask_price;
}