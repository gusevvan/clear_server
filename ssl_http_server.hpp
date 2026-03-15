#pragma once
#include "http_server_base.hpp"

#include <boost/asio/ssl.hpp>

#include <filesystem>
#include <format>
#include <fstream>

namespace clear_server {

namespace ssl = asio::ssl;

class SslHttpServer final : public HttpServerBase<ssl::stream<beast::tcp_stream>> {
public:    
    SslHttpServer(
        const std::string& address, 
        unsigned short port,
        const std::string& cert_path,
        const std::string& key_path)
        : HttpServerBase(address, port)
        , ssl_ctx_{ssl::context::tlsv12} {
        tune_ssl(cert_path, key_path);
    }

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

    void tune_ssl(const std::string& cert_path, const std::string& key_path) {
        ssl_ctx_.set_options(
            boost::asio::ssl::context::default_workarounds | 
            boost::asio::ssl::context::no_sslv2
        );

        auto cert = load_cipher_content(cert_path);
        ssl_ctx_.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

        auto key = load_cipher_content(key_path);
        ssl_ctx_.use_private_key(
            boost::asio::buffer(key.data(), key.size()),
            boost::asio::ssl::context::file_format::pem
        );
    }

    std::string load_cipher_content(const std::string& cipher_path) {
        std::ifstream cipher_file{cipher_path};
        if (!cipher_file) {
            throw std::runtime_error(std::format("Failed to open file {}", cipher_path));
        }
        auto file_size = std::filesystem::file_size(cipher_path);
        std::string cipher(file_size, '\0');
        if (!cipher_file.read(cipher.data(), file_size)) {
            throw std::runtime_error(std::format("Failed to read {} content", cipher_path));
        }
        return cipher;
    }
};

} // namespace clear_server
