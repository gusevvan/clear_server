#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>

#include "logger/concept.hpp"
#include "http_request.hpp"
#include "http_response.hpp"

namespace clear_server {

namespace asio = boost::asio;
namespace http = boost::beast::http;

class Handler {
public:
    virtual asio::awaitable<void> execute() = 0;

protected:
    Handler(http::verb method, std::string path)
        : method_{method}, path_{std::move(path)} {}

    std::shared_ptr<CustomResponse<Handler>> response() {
        return response_;
    }

    std::optional<HttpRequest> request() {
        if (response_) {
            return response_->get_request();
        }
        return std::nullopt;
    }

private:
    http::verb method_;
    std::string path_;
    std::shared_ptr<CustomResponse<Handler>> response_;

    template <typename TcpStream, logger::LoggerType Logger>
    friend class HttpServerBase;

    template <typename HandlerType>
    friend class HandlersStorage;

    template <typename HandlerType>
    friend class CustomResponse;

    std::string path() {
        return path_;
    }

    http::verb method() {
        return method_;
    }

    virtual asio::awaitable<void> operator()(std::shared_ptr<CustomResponse<Handler>> response) {
        response_ = std::move(response);
        co_await execute();
    }
};

} // namespace clear_server 
