#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <string>
#include <map>

namespace clear_server {

namespace asio = boost::asio;
namespace http = boost::beast::http;

template <typename HandlerType>
class HandlersStorage {
public:
    void add_handler(std::shared_ptr<HandlerType> handler) {
        if (!handler) {
            return;
        }
        handlers_.emplace(std::pair{handler->method(), handler->path()}, std::move(handler));
    }

    std::shared_ptr<HandlerType> find_handler(
            http::verb method, const std::string& path) const {
        auto key = std::pair{method, path};
        if (handlers_.contains(key)) {
            return handlers_.at(key);
        }
        return nullptr;
    }

private:
    std::map<std::pair<http::verb, std::string>, std::shared_ptr<HandlerType>> handlers_;
};


} // namespace clear_server
