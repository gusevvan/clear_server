#include "../http_server.hpp"
#include "../ssl_http_server.hpp"

using namespace clear_server;

class AbabaHandler final : public GetHandler {
public:
    AbabaHandler() : GetHandler("/ababa") {}

private:
    asio::awaitable<void> execute() {
        response()->add_header(Header::content_type, "application/json");
        response()->set_body("q123").set_status(200);
        co_return;
    }
};

class BadHandler final : public PostHandler {
public:
    BadHandler() : PostHandler("/bad") {}

private:
    asio::awaitable<void> execute() {
        throw std::runtime_error{"bad"};
        co_return;
    }
};

int main() {

    SslHttpServer server{"0.0.0.0", 8080, "cert.pem", "key.pem"};

    server.add_handler(std::make_shared<AbabaHandler>());
    server.add_handler(std::make_shared<BadHandler>());

    server.run(4);
}
