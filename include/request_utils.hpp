#pragma once
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "ResponseHelper.hpp"
#include "StandardResponse.hpp"

using json = nlohmann::json;
namespace beast = boost::beast;
namespace http = beast::http;

// Helper function to create and send standardized error responses
template <class Body, class Allocator>
auto bad_request(const http::request<Body, http::basic_fields<Allocator>> &req, beast::string_view why)
{
    spdlog::warn("Bad request: {}", why);

    StandardResponse res_struct = create_error_response(400, std::string(why), "Bad Request");
    json res_json = res_struct.to_json();
    std::string response_body = res_json.dump();

    http::response<http::string_body> res{
        http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = response_body;
    res.prepare_payload();
    return res;
}


// Helper function to create and send standardized not found responses
template <class Body, class Allocator>
auto not_found(const http::request<Body, http::basic_fields<Allocator>> &req, beast::string_view target)
{
    spdlog::warn("Resource not found: {}", target);

    StandardResponse res_struct = create_not_found_response(std::string(target));
    json res_json = res_struct.to_json();
    std::string response_body = res_json.dump();

    http::response<http::string_body> res{
        http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = response_body;
    res.prepare_payload();
    return res;
}

// Helper function to create and send standardized server error responses
template <class Body, class Allocator>
auto server_error(const http::request<Body, http::basic_fields<Allocator>> &req, beast::string_view what)
{
    spdlog::error("Server error: {}", what);

    StandardResponse res_struct = create_internal_server_error_response(std::string(what));
    json res_json = res_struct.to_json();
    std::string response_body = res_json.dump();

    http::response<http::string_body> res{
        http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = response_body;
    res.prepare_payload();
    return res;
}

// Additional helper functions can be added here...