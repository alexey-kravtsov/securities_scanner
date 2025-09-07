#include <http.h>

#include <iostream>
#include <stdexcept>
#include <boost/beast/core/stream_traits.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
namespace ip = asio::ip;

using namespace http;

const int HTTP_CLIENT_MAX_ATTEMPTS = 3;

HttpClient::HttpClient(const std::string& a_host) : HttpClient(a_host, {}) {}

HttpClient::HttpClient(const std::string& a_host, const std::string& a_auth) : host {a_host}, auth {a_auth} {}

HttpClient::~HttpClient() {
    try {
        HttpClient::shutdown();
    } catch (...) {

    }
}

HttpClient::HttpClient(HttpClient&& other) 
    : host(std::move(other.host)),
      ssl_socket_stream(std::move(other.ssl_socket_stream)) {}

std::string HttpClient::get(const std::string& path) {
    return this->request(beast::http::verb::get, path, {});
}

std::string HttpClient::post(const std::string& path, const std::string& request) {
    return this->request(beast::http::verb::post, path, request);
}

std::string HttpClient::request(beast::http::verb method, const std::string& path, const std::string& request) {
    if (!ssl_socket_stream.get()) {
        connect();
    }

    beast::http::request<beast::http::string_body> req{ method, path, 11 };
    req.set(beast::http::field::host, host);
    req.set(boost::beast::http::field::content_type, "application/json");
    if (auth.length() > 0) {
        req.set(beast::http::field::authorization, auth);
    }
    req.body() = std::string {request};
    req.prepare_payload();

   beast:: http::response<beast::http::string_body> res;

    for (int attempt = 1; attempt <= HTTP_CLIENT_MAX_ATTEMPTS; attempt++) {
        try {
            beast::http::response<beast::http::string_body> current_res;
            beast::flat_buffer buffer;
            beast::http::write(*ssl_socket_stream, req);
            beast::http::read(*ssl_socket_stream, buffer, current_res);
            res = std::move(current_res);
            break;
        } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;

            if (attempt >= HTTP_CLIENT_MAX_ATTEMPTS) {
                throw e;
            }

            shutdown();
            connect();
        }
    }

    switch (res.result())
    {
        case beast::http::status::ok:
            return res.body();
        case beast::http::status::not_found:
            throw not_found();
        default:
            throw std::runtime_error {"HTTP status: " + std::to_string(res.result_int())};
    }
}


void HttpClient::connect() {
    service = std::make_unique<asio::io_service>();
    ssl::context ctx(ssl::context::sslv23_client);

    ssl_socket_stream = std::make_unique<socket_stream_t>(socket_stream_t { *service, ctx });
    SSL_set_tlsext_host_name(ssl_socket_stream->native_handle(), host.c_str());

    ip::tcp::resolver resolver(*service);
    auto it = resolver.resolve(host, "443");
    asio::connect(ssl_socket_stream->lowest_layer(), it);
    ssl_socket_stream->handshake(ssl::stream_base::handshake_type::client);
}

void HttpClient::shutdown() {
    if (!ssl_socket_stream.get()) {
        return;
    }

    try {
        beast::close_socket(ssl_socket_stream->lowest_layer());
        service->stop();
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ssl_socket_stream.reset();
    service.reset();
}