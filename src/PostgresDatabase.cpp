// PostgresDatabase.cpp
#include "PostgresDatabase.hpp"
#include <iostream>
#include <stdexcept>
#include "spdlog/spdlog.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

PostgresDatabase::PostgresDatabase(const std::string& connectionString)
    : connectionString_(connectionString), conn_(nullptr)
{
    try {
        conn_ = std::make_unique<pqxx::connection>(connectionString_);
        if (!conn_->is_open()) {
            std::cerr << "Failed to open PostgreSQL database connection.\n";
            throw std::runtime_error("Failed to open database connection.");
        }
        spdlog::info("PostgreSQL connection established.");

        // Prepare the statements once during initialization
        pqxx::work txn(*conn_);
        //txn.conn().prepare("get_message_by_id", "SELECT message FROM messages WHERE id = $1");
        //txn.conn().prepare("get_all_data", "SELECT * FROM your_table");  
        txn.commit();

    } catch (const std::exception& e) {
        std::cerr << "PostgreSQL connection error: " << e.what() << std::endl;
        throw;
    }
}

std::string PostgresDatabase::getMessageById(int id) {
    try {
        pqxx::work txn(*conn_);

        pqxx::result result = txn.prepared("get_message_by_id")(id).exec();

        txn.commit();

        if (!result.empty()) {
            return result[0][0].as<std::string>();
        } else {
            return "Message not found.";
        }
    } catch (const std::exception& e) {
        spdlog::error("PostgreSQL query error: {}", e.what());
        return "Database error.";
    }
}

json PostgresDatabase::getData() {
    try {
        pqxx::work txn(*conn_);

        pqxx::result result = txn.prepared("get_all_data").exec();

        txn.commit();

        json data = json::array();

        for (const auto& row : result) {
            json record;
            for (const auto& field : row) {
                record[field.name()] = field.c_str();
            }
            data.push_back(record);
        }

        return data;
    } catch (const std::exception& e) {
        spdlog::error("PostgreSQL getData error: {}", e.what());
        return json::object(); 
    }
}