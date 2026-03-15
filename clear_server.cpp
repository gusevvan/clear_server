#include "http_server.hpp"
#include "ssl_http_server.hpp"

int main() {
    using namespace clear_server;    

    SslHttpServer server{"0.0.0.0", 8080, "cert.pem", "key.pem"};

    server.add_handler("/ababa", []() -> asio::awaitable<CustomResponse> {
        co_return CustomResponse{"{\"hello\": \"world\"}", static_cast<unsigned short>(200)};
    });

    server.add_handler("/bad", []() -> asio::awaitable<CustomResponse> {
        throw std::runtime_error("Bad");
        co_return CustomResponse{"", static_cast<unsigned short>(101)};
    });

    server.run(4);
}
