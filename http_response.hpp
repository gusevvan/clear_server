#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <unordered_map>

#include "http_request.hpp"
#include "handler.hpp"

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;

class CustomResponse {
public:
    CustomResponse(HttpRequest request, const HandlersStorage& handlers_storage)
        : request_{std::move(request)}
        , handler_{handlers_storage.find_handler(request_)} {}

    CustomResponse& set_header(const std::string& key, const std::string& value) {
        headers_.emplace(key, value);
        return *this;
    }

    CustomResponse& set_status(unsigned short status) {
        status_ = static_cast<http::status>(status);
        return *this;
    }

    CustomResponse& set_body(std::string body) {
        body_ = std::move(body);
        return *this;
    }
    
private:
    HttpRequest request_;
    std::optional<Handler> handler_;
    http::status status_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;

    template <typename TcpStream, typename Logger>
    friend class HttpServerBase;

    asio::awaitable<void> get() {
        if (handler_) {
            status_ = http::status::ok;
        } else {
            status_ = http::status::not_found;
            throw std::runtime_error{"Handler for request not found"};
        }
        try {
            co_await handler_.value()(request_, *this);
        } catch (...) {
            status_ = http::status::internal_server_error;
            throw;
        }
    }
    
    http::message_generator message() {
        http::response<http::string_body> res{status_, request_.raw().version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(request_.raw().keep_alive());
        res.body() = std::move(body_);
        res.prepare_payload();
        return res;
    }

};

} // namespace clear_server
