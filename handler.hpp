#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include <map>

#include "http_request.hpp"

namespace clear_server {

namespace asio = boost::asio;
namespace http = boost::beast::http;

class HttpRequest;
class CustomResponse;

using Handler = std::function<asio::awaitable<void>(const HttpRequest&, CustomResponse&)>;

class HandlersStorage {
public:
    void add_handler(http::verb method, const std::string& path, Handler handler) {
        handlers_.emplace(std::pair{method, path}, std::move(handler));
    }

    std::optional<Handler> find_handler(const HttpRequest& req) const {
        auto key = std::pair{req.raw().method(), req.path()};
        if (handlers_.contains(key)) {
            return handlers_.at(key);
        }
        return std::nullopt;
    }

private:
    std::map<std::pair<http::verb, std::string>, Handler> handlers_;
};


} // namespace clear_server
