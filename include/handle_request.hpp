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
#include "csv_loader.hpp"
#include "handler_hello.hpp"
#include "handler_loadcsv.hpp"
#include "handler_db.hpp"
#include "handler_login.hpp"
#include "request_utils.hpp"

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

    // Perform the authorization check
    if (!check_protected_route(req, std::forward<Send>(send), protected_routes, config))
    {
        return;
    }

    if (req.target() == "/login" && req.method() == http::verb::post)
    {
        handle_login_route(std::forward<decltype(req)>(req), send, db);
        return;
    }

    if (req.target() == "/hello")
    {
        handle_hello_route(std::forward<decltype(req)>(req), send);
        return;
    }

    if (req.target() == "/db")
    {
        handle_db_route(std::forward<decltype(req)>(req), send, db);
        return;
    }

    if (req.target().starts_with("/loadcsv/"))
    {
        // Extract the file name after '/loadcsv/'
        std::string target_str = std::string(req.target());
        std::string file_name = target_str.substr(std::string("/loadcsv/").length());

        // Validate the file name if necessary (e.g., check against a list of valid symbols or format)
        if (file_name.empty())
        {
            http::response<http::string_body> res = bad_request(req, "Missing file name.");
            return send(std::move(res));
        }

        // Pass the file name to the route handler
        handle_loadcsv_route(std::forward<decltype(req)>(req), send, db, file_name);
        return;
    }

    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
    {
        spdlog::warn("Illegal request-target: {}", req.target());
        return send(bad_request(req, "Illegal request-target"));
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