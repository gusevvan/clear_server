#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/error_code.hpp>

#include <thread>
#include <map>

#include "handler.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

template <typename TcpStream, typename Logger>
class HttpServerBase {
public:
    HttpServerBase(const std::string& address, unsigned short port, Logger logger) 
        : endpoint_{asio::ip::make_address(address), port}
        , logger_{std::move(logger)} {}

    void run(int num_threads = std::thread::hardware_concurrency()) {
        asio::io_context ioc{num_threads};
        asio::co_spawn(
            ioc,
            start_listen(),
            [this](std::exception_ptr e) {
                handle_error("Listen", std::move(e));
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
        handlers_storage_.add_handler(method, path, std::move(handler));
    }

protected:
    using TcpStreamType = TcpStream;

    virtual TcpStreamType build_stream(asio::ip::tcp::socket&& client_socket) = 0;
    virtual asio::awaitable<void> on_start(TcpStreamType& stream) { co_return; }
    virtual void set_timeout(TcpStreamType& stream) = 0;
    virtual asio::awaitable<void> shutdown(TcpStreamType& stream) = 0;

private:
    asio::ip::tcp::endpoint endpoint_;
    HandlersStorage handlers_storage_;
    Logger logger_;

    asio::awaitable<void> start_listen() {
        auto executor = co_await asio::this_coro::executor;
        auto acceptor = asio::ip::tcp::acceptor{ executor, endpoint_ };
        log_server_start();
        while (true) {
            asio::co_spawn(
                executor,
                do_session(co_await acceptor.async_accept()),
                [this](std::exception_ptr e) {
                    handle_error("Session", std::move(e));
                }
            );
        }
    }

    void log_server_start() {
        logger_.info("Server started on {}:{}", 
            endpoint_.address().to_string(), endpoint_.port());
    }

    void handle_error(const std::string& type, std::exception_ptr e) {
        if (e) {
            try {
                std::rethrow_exception(e);
            } catch (const boost::system::system_error& e) {
                logger_.error("{} error: {}", type, e.code().message());
            } catch (const std::exception& e) {
                logger_.error("{} error: {}", type, e.what());
            }
        }
    }

    asio::awaitable<void> do_session(asio::ip::tcp::socket&& client_socket) {
        auto stream = build_stream(std::move(client_socket));
        beast::flat_buffer buffer;
        co_await on_start(stream);
        co_await handle_packets(stream);
        co_await shutdown(stream);
    }

    asio::awaitable<void> handle_packets(TcpStreamType& stream) {
        try {
            beast::flat_buffer buffer;
            while (true) {
                set_timeout(stream);
                http::request<http::string_body> req;
                co_await http::async_read(stream, buffer, req);
                log_request(req);
                http::message_generator msg = co_await handle_request(std::move(req));
                bool keep_alive = msg.keep_alive();
                co_await beast::async_write(stream, std::move(msg));
                if (!keep_alive) {
                    co_return;
                }
            }
        } catch (const beast::system_error& e) {
            if (e.code() == http::error::end_of_stream ||
                e.code() == asio::error::eof) {
                co_return;
            }
            throw;
        }
    }

    void log_request(const HttpRequest& req) {
        logger_.info("Get request: {} {}", req.raw().method_string(), req.path());
    }

    asio::awaitable<http::message_generator> handle_request(HttpRequest req) {
        CustomResponse response(req, handlers_storage_);
        try {
            co_await response.get();
        } catch (const std::exception& exc) {
            logger_.error("Handle request error: {}", exc.what());
        } catch (...) {
            logger_.error("Unknown handle request error");
        }
        co_return response.message();
    }
};

#define BASE_HANDLER(server, type, endpoint, ...) \
    server.add_handler(http::verb::get, endpoint, \
        [](const HttpRequest& request, CustomResponse& response) -> asio::awaitable<void> { \
            __VA_ARGS__ \
            co_return; \
        })

#define GET_HANDLER(server, endpoint, ...) \
    BASE_HANDLER(server, http::verb::get, endpoint, __VA_ARGS__)


#define POST_HANDLER(server, endpoint, ...) \
    BASE_HANDLER(server, http::verb::post, endpoint, __VA_ARGS__)

} // namespace clear_server
