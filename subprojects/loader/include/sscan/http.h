#ifndef SECURITIES_SCANNER_HTTP_H
#define SECURITIES_SCANNER_HTTP_H

#include <sscan/rate_limiter.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <string>
#include <memory>

using socket_stream_t = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

namespace http {
    
    class HttpClient {
        public:
            HttpClient(const std::string& host);
            HttpClient(const std::string& host, const std::string& auth, const int rps);
            ~HttpClient();

            HttpClient(const HttpClient& other) = delete;
            HttpClient& operator=(const HttpClient& other) = delete;

            HttpClient(HttpClient&& other) = default;
            HttpClient& operator=(HttpClient&& other) = default;

            std::string get(const std::string& path);
            std::string post(const std::string& path, const std::string& request);

            void shutdown();
        private:
            const std::string host;
            const std::string auth;
            std::optional<RateLimiter> rate_limiter;
            std::unique_ptr<boost::asio::io_service> service;
            std::unique_ptr<socket_stream_t> ssl_socket_stream;

            void connect();
            std::string request(boost::beast::http::verb method, const std::string& path, const std::string& request);
    };

    class not_found : public std::exception {};
}

#endif // SECURITIES_SCANNER_HTTP_H