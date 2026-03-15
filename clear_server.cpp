#include "http_server.hpp"
#include "ssl_http_server.hpp"
#include "server_certificate.hpp"

#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

int main() {
    using namespace clear_server;
    
    auto const address = asio::ip::make_address("0.0.0.0");
    auto const port = static_cast<unsigned short>(8080);
    auto const threads = 4;

    asio::io_context ioc{ threads };
    ssl::context ctx{ssl::context::tlsv12};
    load_server_certificate(ctx);

    SslHttpServer server{address, port, std::move(ctx)};

    asio::co_spawn(
        ioc,
        server.run(),
        [](std::exception_ptr e)
        {
            if(e)
            {
                try
                {
                    std::rethrow_exception(e);
                }
                catch(std::exception const& e)
                {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] { ioc.run(); });
    ioc.run();
}
