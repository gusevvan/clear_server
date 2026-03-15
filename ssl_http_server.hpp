#pragma once
#include "http_server_base.hpp"

#include <boost/asio/ssl.hpp>

namespace clear_server {

namespace ssl = asio::ssl;

class SslHttpServer final : public HttpServerBase<ssl::stream<beast::tcp_stream>> {
public:    
    SslHttpServer(
        const asio::ip::address& address, 
        unsigned short port,
        ssl::context ssl_ctx)
        : HttpServerBase(address, port)
        , ssl_ctx_{std::move(ssl_ctx)} {}

    TcpStreamType build_stream(asio::ip::tcp::socket&& client_socket) override {
        return TcpStreamType{std::move(client_socket), ssl_ctx_};
    }

    asio::awaitable<void> on_start(TcpStreamType& stream) { 
        // ?? thread safity
        co_await asio::dispatch(stream.get_executor(), asio::use_awaitable);
        co_await stream.async_handshake(asio::ssl::stream_base::server, asio::use_awaitable);
    }

    void set_timeout(TcpStreamType& stream) override {
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
    }

    asio::awaitable<void> shutdown(TcpStreamType& stream) override {
        co_await stream.async_shutdown(asio::use_awaitable);
    }

private:
    ssl::context ssl_ctx_;
};

} // namespace clear_server
