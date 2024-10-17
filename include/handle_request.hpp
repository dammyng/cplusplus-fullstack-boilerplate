// handle_request.hpp
#ifndef HANDLE_REQUEST_HPP
#define HANDLE_REQUEST_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include "mime_types.hpp"
#include "path_cat.hpp"
#include "utility.hpp"
#include "IDatabase.hpp"
#include "spdlog/spdlog.h"
#include "StandardResponse.hpp"
#include "ResponseHelper.hpp"
#include "CustomFormatter.hpp"
#include "jwt-cpp/jwt.h"
#include "Config.hpp"
#include "auth_helpers.hpp"

using json = nlohmann::json;
namespace beast = boost::beast;
namespace http = beast::http;

// This function produces an HTTP response for the given request.
// The type of the response object depends on the contents of the request,
template <class Body, class Allocator, class Send>
void handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send,
    std::shared_ptr<IDatabase> db)
{
    spdlog::info("Received {} request for {}", std::string(req.method_string()), std::string(req.target()));

    Config &config = Config::getInstance();

    // List of protected routes that require JWT authentication
    std::vector<std::string> protected_routes = {"/api", "/db"};

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
            return send(std::move(res));
        }

        std::string token = token_opt.value();

        // Verify the JWT token
        bool is_valid = verify_jwt(token, config.jwt_secret, config.jwt_issuer);
        if (!is_valid)
        {
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
            return send(std::move(res));
        }

        spdlog::info("JWT verification successful for request to {}", req.target());
    }

    // Helper function to create and send standardized error responses
    auto const bad_request = [&](beast::string_view why)
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
    };

    auto const not_found = [&](beast::string_view target)
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
    };

    auto const server_error = [&](beast::string_view what)
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
    };

    if (req.method() != http::verb::get && req.method() != http::verb::head)
    {
        spdlog::warn("Unsupported HTTP method: {}", req.method_string());

        StandardResponse res_struct = create_error_response(400, "Unsupported HTTP method", "Bad Request");
        json res_json = res_struct.to_json();
        std::string response_body = res_json.dump();

        http::response<http::string_body> res{
            http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = response_body;
        res.prepare_payload();

        spdlog::info("Bad request response sent");
        return send(std::move(res));
    }

    // **Handle the /login route**
    if (req.target() == "/login" && req.method() == http::verb::post)
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
            http::response<http::string_body> res = bad_request("Invalid JSON format.");
            return send(std::move(res));
        }

        // Extract username and password
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
            http::response<http::string_body> res = bad_request("Missing username or password.");
            return send(std::move(res));
        }

        // Authenticate user (this is a placeholder; implement actual authentication logic)
        // bool is_authenticated = db->authenticate_user(username, password); 
        bool is_authenticated = true; // Always true in mock

        if (!is_authenticated)
        {
            spdlog::warn("Authentication failed for user: {}", username);
            // Respond with 401 Unauthorized
            StandardResponse res_struct = create_error_response(401, "Invalid credentials.", "Unauthorized");
            json res_json = res_struct.to_json();
            std::string response_body = res_json.dump();

            http::response<http::string_body> res{
                http::status::unauthorized, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = response_body;
            res.prepare_payload();
            return send(std::move(res));
        }

        // Create JWT token
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
        std::string response_body = res_json.dump();

        http::response<http::string_body> res{
            http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = response_body;
        res.prepare_payload();
        spdlog::info("/login response sent");
        return send(std::move(res));
    }

    if (req.target() == "/hello")
    {
        spdlog::info("Handling /hello route");

        json data = {
            {"message", "Hello worlded"}};

        // Create a standardized success response
        StandardResponse res_struct = create_success_response(200, data);
        json res_json = res_struct.to_json();

        std::string response_body = res_json.dump();

        http::response<http::string_body> res{
            http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = response_body;
        res.prepare_payload();
        spdlog::info("/hello response sent");
        return send(std::move(res));
    }

    // **Handle the /db route**
    if (req.target() == "/db")
    {
        spdlog::info("Handling /db route");
        try
        {
            json data = db->getData(); 
            spdlog::info("Database returned data");

            StandardResponse res_struct = create_success_response(200, data);
            json res_json = res_struct.to_json();

            std::string response_body = res_json.dump();

            http::response<http::string_body> res{
                http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = response_body;
            res.prepare_payload();
            spdlog::info("/db response sent");
            return send(std::move(res));
        }
        catch (const std::exception &e)
        {
            spdlog::error("Database error: {}", e.what());

            StandardResponse res_struct = create_internal_server_error_response(e.what());
            json res_json = res_struct.to_json();
            std::string response_body = res_json.dump();

            http::response<http::string_body> res{
                http::status::internal_server_error, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = response_body;
            res.prepare_payload();
            spdlog::info("/db error response sent");
            return send(std::move(res));
        }
    }

    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
    {
        spdlog::warn("Illegal request-target: {}", req.target());
        return send(bad_request("Illegal request-target"));
    }

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if (req.target().back() == '/')
        path.append("index.html");

    spdlog::info("Serving file: {}", path);

    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    if (ec == beast::errc::no_such_file_or_directory)
    {
        spdlog::warn("File not found: {}", path);
        return send(not_found(req.target()));
    }

    if (ec == beast::errc::no_such_file_or_directory)
    {
        spdlog::warn("File not found: {}", path);

        StandardResponse res_struct = create_not_found_response(std::string(req.target()));
        json res_json = res_struct.to_json();
        std::string response_body = res_json.dump();

        http::response<http::string_body> res{
            http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = response_body;
        res.prepare_payload();
        return send(std::move(res));
    }

    // Handle an unknown error
    if (ec)
    {
        spdlog::error("Error opening file {}: {}", path, ec.message());

        StandardResponse res_struct = create_server_error_response(ec.message());
        json res_json = res_struct.to_json();
        std::string response_body = res_json.dump();

        http::response<http::string_body> res{
            http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = response_body;
        res.prepare_payload();
        return send(std::move(res));
    }

    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{
            http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        spdlog::info("HEAD response sent for {}", path);
        return send(std::move(res));
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    spdlog::info("GET response sent for {}", path);
    return send(std::move(res));
}

#endif 