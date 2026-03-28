#include "../ssl_http_server.hpp"

using namespace clear_server;

class AbabaHandler final : public GetHandler {
public:
    AbabaHandler() : GetHandler("/ababa") {}

private:
    asio::awaitable<void> execute() {
        response()->set_body("handled").set_status(201);
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
    using namespace clear_server;    

    SslHttpServer server{"0.0.0.0", 8080, "cert.pem", "key.pem"};

    server.add_handler(std::make_shared<AbabaHandler>());
    server.add_handler(std::make_shared<BadHandler>());

    server.run(4);
}
