#pragma once
#include "http_server_base.hpp"

namespace clear_server {

class HttpServer final : public HttpServerBase<beast::tcp_stream> {
public:
    HttpServer(const asio::ip::address& address, unsigned short port)
        : HttpServerBase(address, port) {}

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
