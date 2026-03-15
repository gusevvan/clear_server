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
#include <unordered_map>

#include "http_utils.hpp"

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

template <typename TcpStream>
class HttpServerBase {
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

    void add_handler(const std::string& path, std::function<asio::awaitable<std::string>()> handler) {
        handlers_.emplace(path, std::move(handler));
    }

protected:
    using TcpStreamType = TcpStream;

    virtual TcpStreamType build_stream(asio::ip::tcp::socket&& client_socket) = 0;
    virtual asio::awaitable<void> on_start(TcpStreamType& stream) { co_return; }
    virtual void set_timeout(TcpStreamType& stream) = 0;
    virtual asio::awaitable<void> shutdown(TcpStreamType& stream) = 0;

private:
    asio::ip::tcp::endpoint endpoint_;
    std::unordered_map<std::string, std::function<asio::awaitable<std::string>()>> handlers_;

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
        auto status = http::status::not_found;
        std::string payload;
        if (handlers_.contains(req.path())) {
            status = http::status::ok;
            const auto& handler = handlers_.at(req.path());
            payload = co_await handler();
        }
        http::response<http::string_body> res{status, req.raw().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.raw().keep_alive());
        res.body() = payload;
        res.prepare_payload();
        co_return res;
    }
};

} // namespace clear_server
