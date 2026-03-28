#pragma once
#include <boost/beast/http.hpp>
#include <boost/url/parse_query.hpp>
#include <boost/url/url_view.hpp>

#include <unordered_map>

#include "logger/concept.hpp"

namespace clear_server {

namespace http = boost::beast::http;
namespace url = boost::urls;

class HttpRequest {

    using Params = std::unordered_map<std::string, std::string>;

public:
    HttpRequest(http::request<http::string_body> beast_req) 
        : beast_req_{std::move(beast_req)}
        , target_(beast_req_.target()) {}

    Params query_params() const {
        Params query_params;
        for (const auto& [key, value, placeholder] : target_.params()) {
            query_params.emplace(key, value);
        }
        return query_params;
    }

    std::string path() const {
        return target_.path();
    }

    std::string body() const {
        return beast_req_.body();
    }

    Params headers() const {
        Params headers;
        for (const auto& field : beast_req_) {
            headers.emplace(field.name_string(), field.value());
        }
        return headers;
    }

private:
    http::request<http::string_body> beast_req_; 
    url::url_view target_;

    template <typename TcpStream, logger::LoggerType Logger>
    friend class HttpServerBase;

    friend class HandlersStorage;
    friend class CustomResponse;

    const http::request<http::string_body>& raw() const {
        return beast_req_;
    }
};

} // namespace clear_server
