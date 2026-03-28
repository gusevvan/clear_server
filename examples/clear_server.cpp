#include "../http_server.hpp"
#include "../ssl_http_server.hpp"

using namespace clear_server;

class AbabaHandler final : public Handler {
public:
    AbabaHandler() : Handler(http::verb::get, "/ababa") {}

private:
    asio::awaitable<void> execute() {
        for (const auto& [key, value] : request()->headers()) {
            std::cout << key << " " << value << std::endl;
        }
        response()->set_body("q123").set_status(200);
        co_return;
    }
};

int main() {

    HttpServer server{"0.0.0.0", 8080};

    server.add_handler(std::make_shared<AbabaHandler>());

    server.run(4);
}
