#pragma once
#include <boost/asio/awaitable.hpp>
#include <functional>
#include <string>

#include "http_utils.hpp"

namespace clear_server {

namespace asio = boost::asio;

class Handler {
public:
    Handler(std::function<asio::awaitable<CustomResponse>(Handler&)> handler)
        : handler_{std::move(handler)} {}

    asio::awaitable<CustomResponse> operator()(HttpRequest req) {
        body_ = req.raw().body();
        for (const auto& [key, value, type] : req.query_params()) {
            query_params_.emplace(key, value);
        }
        co_await handler_(*this);
    }

    const std::string& body() {
        return body_;
    }

    const std::unordered_map<std::string, std::string>& query_params() {
        return query_params_;
    }

private:
    std::function<asio::awaitable<CustomResponse>(Handler&)> handler_;
    std::string body_;
    std::unordered_map<std::string, std::string> query_params_;
};

} // namespace clear_server
