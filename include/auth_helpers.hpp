#ifndef AUTH_HELPERS_HPP
#define AUTH_HELPERS_HPP

#include <boost/beast/http.hpp>
#include "jwt-cpp/jwt.h"
#include "Config.hpp"
#include "spdlog/spdlog.h"
#include <optional>

namespace http = boost::beast::http;

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

#endif