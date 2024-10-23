#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include "StandardResponse.hpp"
#include "IDatabase.hpp"
#include "ResponseHelper.hpp"
#include "spdlog/spdlog.h"
#include "auth_helpers.hpp"
#include "request_utils.hpp"

#include "jwt-cpp/jwt.h"
#include "Config.hpp"

using json = nlohmann::json;

template <class Body, class Allocator, class Send>
void handle_login_route(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send,
    std::shared_ptr<IDatabase> db)
{
    spdlog::info("Handling /login route");

    json request_json;
    try
    {
        request_json = json::parse(req.body());
    }
    catch (const std::exception &e)
    {
        spdlog::warn("Invalid JSON in /login request: {}", e.what());
        http::response<http::string_body> res = bad_request(req, "Invalid JSON format.");
        //http::response<http::string_body> res = bad_request("Invalid JSON format.");
        return send(std::move(res));
    }

    std::string username;
    std::string password;
    try
    {
        username = request_json.at("username").get<std::string>();
        password = request_json.at("password").get<std::string>();
    }
    catch (const std::exception &e)
    {
        spdlog::warn("Missing username or password in /login request: {}", e.what());
        http::response<http::string_body> res = bad_request(req, "Missing username or password.");

        //http::response<http::string_body> res = bad_request("Missing username or password.");
        return send(std::move(res));
    }

    // Authenticate user (replace with real authentication logic)
    //bool is_authenticated = db->authenticate_user(username, password); // Placeholder
    // Mock
    bool is_authenticated = true;

    if (!is_authenticated)
    {
        spdlog::warn("Authentication failed for user: {}", username);
        StandardResponse res_struct = create_error_response(401, "Invalid credentials.", "Unauthorized");
        json res_json = res_struct.to_json();
        http::response<http::string_body> res{
            http::status::unauthorized, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = res_json.dump();
        res.prepare_payload();
        return send(std::move(res));
    }

    Config &config = Config::getInstance();
    auto token = jwt::create()
                     .set_issuer(config.jwt_issuer)
                     .set_type("JWS")
                     .set_subject(username)
                     .set_issued_at(std::chrono::system_clock::now())
                     .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(config.jwt_expiration))
                     .sign(jwt::algorithm::hs256{config.jwt_secret});

    spdlog::info("JWT token created for user: {}", username);

    json data = {
        {"token", token}};

    StandardResponse res_struct = create_success_response(200, data);
    json res_json = res_struct.to_json();
    http::response<http::string_body> res{
        http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = res_json.dump();
    res.prepare_payload();
    spdlog::info("/login response sent");
    return send(std::move(res));
}