#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <nlohmann/json.hpp>
#include "StandardResponse.hpp"
#include <spdlog/spdlog.h>
#include "IDatabase.hpp"
#include "csv_loader.hpp"
#include "StockPrice.hpp"
#include "ResponseHelper.hpp"

using json = nlohmann::json;

template <class Body, class Allocator, class Send>
void handle_db_route(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send,
    std::shared_ptr<IDatabase> db)
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