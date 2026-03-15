#include "http_server.hpp"
#include "ssl_http_server.hpp"

int main() {
    using namespace clear_server;    

    SslHttpServer server{"0.0.0.0", 8080, "cert.pem", "key.pem"};

    server.add_handler("/ababa", []() -> asio::awaitable<std::string> {
        co_return "{\"hello\": \"world\"}";
    });

    server.run(4);
}
