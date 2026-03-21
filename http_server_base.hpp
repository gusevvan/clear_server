#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <iostream>
#include <thread>
#include <map>

#include "http_utils.hpp"

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

template <typename TcpStream>
class HttpServerBase {

    using Handler = std::function<asio::awaitable<CustomResponse>(const HttpRequest&)>;

public:
    HttpServerBase(const std::string& address, unsigned short port) 
        : endpoint_{asio::ip::make_address(address), port} {}

    void run(int num_threads = std::thread::hardware_concurrency()) {
        asio::io_context ioc{num_threads};
        asio::co_spawn(
            ioc,
            start_listen(),
            [](std::exception_ptr e) {
                if (e) {
                    try {
                        std::rethrow_exception(e);
                    } catch(std::exception const& e) {
                        std::cerr << "Error: " << e.what() << std::endl;
                    }
                }
            }
        );
        std::vector<std::thread> io_pool;
        io_pool.reserve(num_threads - 1);
        for (auto i = num_threads - 1; i > 0; --i) {
            io_pool.emplace_back([&ioc] { ioc.run(); });
        }
        ioc.run();
    }

    void add_handler(http::verb method, const std::string& path, Handler handler) {
        handlers_.emplace(std::pair{method, path}, std::move(handler));
    }

protected:
    using TcpStreamType = TcpStream;

    virtual TcpStreamType build_stream(asio::ip::tcp::socket&& client_socket) = 0;
    virtual asio::awaitable<void> on_start(TcpStreamType& stream) { co_return; }
    virtual void set_timeout(TcpStreamType& stream) = 0;
    virtual asio::awaitable<void> shutdown(TcpStreamType& stream) = 0;

private:
    asio::ip::tcp::endpoint endpoint_;
    std::map<std::pair<http::verb, std::string>, Handler> handlers_;

    asio::awaitable<void> start_listen() {
        auto executor = co_await asio::this_coro::executor;
        auto acceptor = asio::ip::tcp::acceptor{ executor, endpoint_ };
        while (true) {
            asio::co_spawn(
                executor,
                do_session(co_await acceptor.async_accept()),
                [](std::exception_ptr e) {
                    if (e) {
                        try {
                            std::rethrow_exception(e);
                        } catch(std::exception const& e) {
                            std::cerr << "Error in session: " << e.what() << "\n";
                        }
                    }
                }
            );
        }
    }

    asio::awaitable<void> do_session(asio::ip::tcp::socket&& client_socket) {
        auto stream = build_stream(std::move(client_socket));
        beast::flat_buffer buffer;
        co_await on_start(stream);
        while (true) {
            set_timeout(stream);
            http::request<http::string_body> req;
            co_await http::async_read(stream, buffer, req);
            http::message_generator msg = co_await handle_request(std::move(req));
            bool keep_alive = msg.keep_alive();
            co_await beast::async_write(stream, std::move(msg));
            if (!keep_alive) {
                break;
            }
        }
        co_await shutdown(stream);
    }

    asio::awaitable<http::message_generator> handle_request(HttpRequest req) {
        CustomResponse handle_res;
        if (handlers_.contains(std::pair{req.raw().method(), req.path()})) {
            handle_res.status() = http::status::internal_server_error;
            try {
                const auto& handler = handlers_.at(std::pair{req.raw().method(), req.path()});
                handle_res = co_await handler(req);
            } catch (std::exception& exc) {
                std::cerr << std::format("Error while handling {}: {}", 
                    req.path(), exc.what()) << std::endl; 
            } catch (...) {
                std::cerr << std::format("Unknown error while handling {}", 
                    req.path()) << std::endl;
            }
        } else {
            handle_res.status() = http::status::not_found;
        }
        http::response<http::string_body> res{handle_res.status(), req.raw().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.raw().keep_alive());
        res.body() = std::move(handle_res).payload();
        res.prepare_payload();
        co_return res;
    }
};

#define BASE_HANDLER(server, type, endpoint, ...) \
    server.add_handler(http::verb::get, endpoint, \
        [](const HttpRequest& request) -> asio::awaitable<CustomResponse> __VA_ARGS__ )

#define GET_HANDLER(server, endpoint, ...) \
    BASE_HANDLER(server, http::verb::get, endpoint, __VA_ARGS__)


#define POST_HANDLER(server, endpoint, ...) \
    BASE_HANDLER(server, http::verb::post, endpoint, __VA_ARGS__)

} // namespace clear_server
