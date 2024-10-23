#ifndef AUTH_HELPERS_HPP
#define AUTH_HELPERS_HPP

#include <boost/beast/http.hpp>
#include "jwt-cpp/jwt.h"
#include "Config.hpp"
#include "spdlog/spdlog.h"
#include <optional>
#include <vector>
#include <string>
#include <nlohmann/json.hpp> // Assuming you're using nlohmann for JSON handling

namespace http = boost::beast::http;
using json = nlohmann::json;

// Extract Bearer token from the Authorization header
inline std::optional<std::string> extract_bearer_token(const http::request<http::string_body>& req) {
    auto auth_iter = req.find(http::field::authorization);
    if (auth_iter != req.end()) {
        std::string auth_header = auth_iter->value().to_string();
        std::string prefix = "Bearer ";
        if (auth_header.compare(0, prefix.size(), prefix) == 0) {
            return auth_header.substr(prefix.size());
        }
    }
    return std::nullopt;
}

// Verify the provided JWT token with the given secret and issuer
inline bool verify_jwt(const std::string& token, const std::string& secret, const std::string& issuer) {
    try {
        auto decoded = jwt::decode(token);

        // Verify the algorithm and issuer
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer(issuer)
            .verify(decoded);

        return true;
    } catch (const std::exception& e) {
        spdlog::warn("JWT verification failed: {}", e.what());
        return false;
    }
}

// Check if a request is to a protected route and perform JWT authentication
template <class Send>
bool check_protected_route(
    const http::request<http::string_body> &req,
    Send &&send,
    const std::vector<std::string> &protected_routes,
    const Config &config)
{
    // Check if the requested route is protected
    bool is_protected = false;
    for (const auto &route : protected_routes)
    {
        if (req.target().starts_with(route))
        {
            is_protected = true;
            break;
        }
    }

    if (is_protected)
    {
        // Extract the Bearer token from the Authorization header
        auto token_opt = extract_bearer_token(req);
        if (!token_opt.has_value())
        {
            spdlog::warn("Missing or malformed Authorization header.");
            // Respond with 401 Unauthorized
            StandardResponse res_struct = create_error_response(401, "Missing or malformed Authorization header.", "Unauthorized");
            json res_json = res_struct.to_json();
            std::string response_body = res_json.dump();

            http::response<http::string_body> res{
                http::status::unauthorized, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = response_body;
            res.prepare_payload();
            send(std::move(res));
            return false;
        }

        std::string token = token_opt.value();

        // Verify the JWT token
        bool is_valid = verify_jwt(token, config.jwt_secret, config.jwt_issuer);
        if (!is_valid)
        {
            spdlog::warn("Invalid or expired JWT token.");
            // Respond with 403 Forbidden
            StandardResponse res_struct = create_error_response(403, "Invalid or expired token.", "Forbidden");
            json res_json = res_struct.to_json();
            std::string response_body = res_json.dump();

            http::response<http::string_body> res{
                http::status::forbidden, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = response_body;
            res.prepare_payload();
            send(std::move(res));
            return false;
        }

        spdlog::info("JWT verification successful for request to {}", req.target());
    }
    return true; // Authorized or not a protected route
}

#endif // AUTH_HELPERS_HPP