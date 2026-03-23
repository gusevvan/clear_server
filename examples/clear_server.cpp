#include "../http_server.hpp"
#include "../ssl_http_server.hpp" 

int main() {
    using namespace clear_server;    

    SslHttpServer server{"0.0.0.0", 8080, "cert.pem", "key.pem"};

    GET_HANDLER(server, "/ababa", {
        for (const auto& [key, value] : request.headers()) {
            std::cout << key << " " << value << std::endl;
        }
        response.set_body("q123").set_status(200);
    });

    POST_HANDLER(server, "/bad", {
        throw std::runtime_error("Bad");
    });

    server.run(4);
}
