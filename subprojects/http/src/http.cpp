#include <http.h>
#include <iostream>
#include <stdexcept>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace ip = asio::ip;
namespace http = beast::http;

HttpClient::HttpClient() {

}

HttpClient::~HttpClient() {
    try {
        HttpClient::shutdown();
    } catch (...) {

    }
}

HttpClient::HttpClient(const HttpClient&& other) {
    HttpClient::host = other.host;
    HttpClient::socket_ptr = std::unique_ptr<socket_t>(other.socket_ptr);
}

void HttpClient::connect(const std::string& host) {
    asio::io_service svc;
    ssl::context ctx(ssl::context::sslv23_client);

    socket_t* ssocket = new socket_t { svc, ctx };
    HttpClient::socket_ptr = std::unique_ptr<socket_t>(ssocket);
    SSL_set_tlsext_host_name(ssocket->native_handle(), host.c_str());

    ip::tcp::resolver resolver(svc);
    auto it = resolver.resolve(host, "443");
    asio::connect(ssocket->lowest_layer(), it);
    ssocket->handshake(ssl::stream_base::handshake_type::client);
}

std::string HttpClient::get(const std::string& path) {
    socket_t* ssocket = HttpClient::socket_ptr.get();
    if (!ssocket) {
        throw std::logic_error("Not connected");
    }

    http::request<http::string_body> req{ http::verb::get, path, 11 };
    req.set(http::field::host, host);
    http::write(ssocket, req);
    http::response<http::string_body> res;
    beast::flat_buffer buffer;
    http::read(ssocket, buffer, res);
    std::cout << "Headers" << std::endl;
    std::cout << res.base() << std::endl << std::endl;
    std::cout << "Body" << std::endl;
    std::cout << res.body() << std::endl << std::endl;
    ssocket.shutdown();
}

void HttpClient::shutdown() {

}