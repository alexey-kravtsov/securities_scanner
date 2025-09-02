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


HttpClient::HttpClient(HttpClient&& other) 
    : host(std::move(other.host)),
      ssl_socket(std::move(other.ssl_socket)) {}


void HttpClient::connect(const std::string& host) {
    this->host = host;

    asio::io_service svc;
    ssl::context ctx(ssl::context::sslv23_client);

    ssl_socket = std::make_unique<socket_t>(socket_t { svc, ctx });
    SSL_set_tlsext_host_name(ssl_socket->native_handle(), host.c_str());

    ip::tcp::resolver resolver(svc);
    auto it = resolver.resolve(host, "443");
    asio::connect(ssl_socket->lowest_layer(), it);
    ssl_socket->handshake(ssl::stream_base::handshake_type::client);
}

std::string HttpClient::get(const std::string& path) {
    if (!ssl_socket.get()) {
        throw std::logic_error("Not connected");
    }

    http::request<http::string_body> req{ http::verb::get, path, 11 };
    req.set(http::field::host, host);
    http::write(*ssl_socket, req);
    http::response<http::string_body> res;
    beast::flat_buffer buffer;
    http::read(*ssl_socket, buffer, res);
    std::cout << "Headers" << std::endl;
    std::cout << res.base() << std::endl << std::endl;
    std::cout << "Body" << std::endl;
    std::cout << res.body() << std::endl << std::endl;

    return res.body();
}

void HttpClient::shutdown() {
    if (!ssl_socket.get()) {
        return;
    }

    try {
        ssl_socket->shutdown();
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ssl_socket.reset();
}