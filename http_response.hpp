#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <unordered_map>

#include "http_request.hpp"
#include "handlers_storage.hpp"
#include "logger/concept.hpp"

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;
using Header = http::field;

template <typename HandlerType>
class CustomResponse {
public:
    
    CustomResponse(HttpRequest request, const HandlersStorage<HandlerType>& handlers_storage)
        : request_{std::move(request)} {
        handler_ = handlers_storage.find_handler(
            request_.raw().method(), request_.path()
        );
        add_header(Header::server, BOOST_BEAST_VERSION_STRING);
        add_header(Header::content_type, "text/plain");
    }

    CustomResponse& add_header(Header name, std::string value) {
        headers_[name] = std::move(value);
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

    HttpRequest get_request() {
        return request_;
    }
    
private:
    HttpRequest request_;
    std::shared_ptr<HandlerType> handler_;
    http::status status_;
    std::string body_;
    std::unordered_map<Header, std::string> headers_;

    template <typename TcpStream, logger::LoggerType Logger>
    friend class HttpServerBase;

    asio::awaitable<void> get(std::shared_ptr<CustomResponse<HandlerType>> self) {
        if (handler_) {
            status_ = http::status::ok;
        } else {
            status_ = http::status::not_found;
            throw std::runtime_error{"Handler for request not found"};
        }
        try {
            co_await (*handler_)(std::move(self));
        } catch (...) {
            status_ = http::status::internal_server_error;
            throw;
        }
    }
    
    http::message_generator message() {
        http::response<http::string_body> res{status_, request_.raw().version()};
        for (const auto& [name, value] : headers_) {
            res.set(name, value);
        }
        res.keep_alive(request_.raw().keep_alive());
        res.body() = std::move(body_);
        res.prepare_payload();
        return res;
    }

};

} // namespace clear_server
