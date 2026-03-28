#pragma once
#include "http_server_base.hpp"
#include "logger/concept.hpp"
#include "logger/logger.hpp"

namespace clear_server {

template <logger::LoggerType Logger = logger::DefaultLogger>
class HttpServer final : public HttpServerBase<beast::tcp_stream, Logger> {
private:
    using TcpStreamType = typename HttpServerBase<beast::tcp_stream, Logger>::TcpStreamType;
public:
    HttpServer(const std::string& address, unsigned short port, Logger logger = {})
        : HttpServerBase<beast::tcp_stream, Logger>(address, port, std::move(logger)) {}

private:
    TcpStreamType build_stream(asio::ip::tcp::socket&& client_socket) override {
        return TcpStreamType{std::move(client_socket)};
    }

    void set_timeout(TcpStreamType& stream) override {
        stream.expires_after(std::chrono::seconds(30));
    }

    asio::awaitable<void> shutdown(TcpStreamType& stream) override {
        stream.socket().shutdown(asio::ip::tcp::socket::shutdown_send);
        co_return;
    }
};

} // namespace clear_server
