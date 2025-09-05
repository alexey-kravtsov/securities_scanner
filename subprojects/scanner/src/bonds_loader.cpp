#include <bonds_loader.h>

#include <http.h>
#include <iostream>
#include <regex>

const std::regex isin_link_r(R"()");

std::vector<BondInfo> BondsLoader::load() {
    auto sl_client = HttpClient();
    sl_client.connect("");

    for (int page = 1; page <= 100; page++) {
        auto path = std::format("", std::to_string(page));
        auto response = sl_client.get(path);

        std::sregex_iterator it(response.begin(), response.end(), isin_link_r);
        std::sregex_iterator end;

        if (it == end) {
            break;
        }

        for (; it != end; it++) {
            std::smatch match = *it;
            std::cout << match.str() << std::endl;
        }
    }

    sl_client.shutdown();

    std::cout << "bonds loaded" << std::endl;

    return std::vector<BondInfo>();
}