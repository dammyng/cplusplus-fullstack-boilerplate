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
void handle_loadcsv_route(
    http::request<Body, http::basic_fields<Allocator>> &&req,
    Send &&send,
    std::shared_ptr<IDatabase> db, const std::string &file_name)
{
    spdlog::info("Handling /loadcsv route for file: {}", file_name);
    try
    {
        // Create a file path based on the file name
        std::string file_path = "../data/" + file_name + ".csv";

        // Load the CSV content using the separated function and StockPrice mapping
        std::vector<StockPrice> stock_data = load_csv<StockPrice>(file_path, map_to_stock_price);
        json response_data = stock_data;

        StandardResponse res_struct = create_success_response(200, response_data);
        json res_json = res_struct.to_json();

        http::response<http::string_body> res{
            http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = res_json.dump();
        res.prepare_payload();

        spdlog::info("/loadcsv response sent");
        return send(std::move(res));
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error handling /loadcsv route: {}", e.what());

        http::response<http::string_body> res{
            http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "Internal Server Error";
        res.prepare_payload();
        return send(std::move(res));
    }
}