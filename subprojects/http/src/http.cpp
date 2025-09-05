#include <http.h>

#include <iostream>
#include <stdexcept>
#include <boost/beast/core/stream_traits.hpp>

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
      ssl_socket_stream(std::move(other.ssl_socket_stream)) {}


void HttpClient::connect(const std::string& host) {
    this->host = host;

    asio::io_service svc;
    ssl::context ctx(ssl::context::sslv23_client);

    ssl_socket_stream = std::make_unique<socket_stream_t>(socket_stream_t { svc, ctx });
    SSL_set_tlsext_host_name(ssl_socket_stream->native_handle(), host.c_str());

    ip::tcp::resolver resolver(svc);
    auto it = resolver.resolve(host, "443");
    asio::connect(ssl_socket_stream->lowest_layer(), it);
    ssl_socket_stream->handshake(ssl::stream_base::handshake_type::client);
}

std::string HttpClient::get(const std::string& path) {
    if (!ssl_socket_stream.get()) {
        throw std::logic_error("Not connected");
    }

    http::request<http::string_body> req{ http::verb::get, path, 11 };
    req.set(http::field::host, host);
    http::write(*ssl_socket_stream, req);
    http::response<http::string_body> res;
    beast::flat_buffer buffer;
    http::read(*ssl_socket_stream, buffer, res);

    return res.body();
}

void HttpClient::shutdown() {
    if (!ssl_socket_stream.get()) {
        return;
    }

    try {
        beast::close_socket(ssl_socket_stream->lowest_layer());
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ssl_socket_stream.reset();
}