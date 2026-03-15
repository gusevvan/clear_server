#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/url/parse_query.hpp>
#include <boost/url/url_view.hpp>

#include <unordered_map>

namespace clear_server {

namespace beast = boost::beast;
namespace http = beast::http;
namespace url = boost::urls;

class HttpRequest {

    using QueryParams = std::unordered_map<std::string, std::string>;

public:
    HttpRequest(http::request<http::string_body> beast_req) 
        : beast_req_{std::move(beast_req)}
        , target_(beast_req_.target()) {}

    auto query_params() {
        return target_.params();
    }

    std::string path() {
        return target_.path();
    }

    const http::request<http::string_body>& raw() {
        return beast_req_;
    }

private:
    http::request<http::string_body> beast_req_; 
    url::url_view target_;
};

} // namespace clear_server
