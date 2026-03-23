#include "../http_server.hpp"

int main() {
    using namespace clear_server;    

    HttpServer server{"0.0.0.0", 8080};

    GET_HANDLER(server, "/ababa", {
        response.set_body("handled").set_status(200);
    });

    POST_HANDLER(server, "/bad", {
        throw std::runtime_error("Bad");
    });

    server.run(4);
}
