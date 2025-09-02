#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include <string>
#include <memory>

using socket_t = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        HttpClient(const HttpClient& other) = delete;
        HttpClient& operator=(const HttpClient& other) = delete;

        HttpClient(const HttpClient&& other);
        HttpClient& operator=(HttpClient&& other);

        void connect(const std::string& host);
        std::string get(const std::string& path);
        void shutdown();
    private:
        std::string host;
        std::unique_ptr<socket_t> socket_ptr;
};