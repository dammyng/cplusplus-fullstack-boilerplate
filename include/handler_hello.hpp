#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "StandardResponse.hpp"

using json = nlohmann::json;

template <class Body, class Allocator, class Send>
void handle_hello_route(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send)
{
    spdlog::info("Handling /hello route");

    json data = {
        {"message", "Hello world why"}};

    StandardResponse res_struct = create_success_response(200, data);
    json res_json = res_struct.to_json();

    http::response<http::string_body> res{
        http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = res_json.dump();
    res.prepare_payload();

    spdlog::info("/hello response sent");
    return send(std::move(res));
}