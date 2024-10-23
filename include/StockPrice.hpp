#ifndef STOCK_PRICE_HPP
#define STOCK_PRICE_HPP

#include <string>
#include <map>
#include <stdexcept>
#include <iostream> // For logging
#include <algorithm> // For std::find_if
#include "spdlog/spdlog.h"

// Define StockPrice struct
struct StockPrice {
    std::string Date;
    double Price;
    double Open;
    double High;
    double Low;
    std::string Volume;
    double ChangePercent;
};

// Helper function to remove surrounding quotes from a string
inline std::string strip_quotes(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

// Define how to convert StockPrice to JSON using nlohmann::json
inline void to_json(nlohmann::json& j, const StockPrice& stock) {
    j = nlohmann::json{
        {"Date", stock.Date},
        {"Price", stock.Price},
        {"Open", stock.Open},
        {"High", stock.High},
        {"Low", stock.Low},
        {"Volume", stock.Volume},
        {"ChangePercent", stock.ChangePercent}
    };
}

inline StockPrice map_to_stock_price(const std::map<std::string, std::string>& row) {
    StockPrice stock;


    // Log the available keys in the current row for debugging
    spdlog::info("Keys in the current row:");
    for (const auto& [key, value] : row) {
        spdlog::info("  Key: '{}', Length: {}", key, key.size());
        for (size_t i = 0; i < key.size(); ++i) {
            spdlog::info("    Char {}: '{}' (ASCII: {})", i, key[i], static_cast<int>(key[i]));
        }
    }

    // Helper function to retrieve a value, strip quotes, or throw a meaningful error
    auto get_value = [&row](const std::string& key) -> std::string {
        auto stripped_key = strip_quotes(key);

        // Log the stripped and original keys for verification
        spdlog::info("Searching for key: '{}', after stripping: '{}', Length: {}", key, key, key.size());
        for (size_t i = 0; i < stripped_key.size(); ++i) {
            spdlog::info("    Char {}: '{}' (ASCII: {})", i, key[i], static_cast<int>(key[i]));
        }

        auto it = row.find(stripped_key);
        if (it == row.end()) {
            // Log all keys present in the row for detailed comparison
            spdlog::error("Could not find stripped key '{}'. Available keys are:", stripped_key);
            for (const auto& [available_key, _] : row) {
                spdlog::error("  Available Key: '{}', Length: {}", available_key, available_key.size());
                for (size_t i = 0; i < available_key.size(); ++i) {
                    spdlog::info("    Char {}: '{}' (ASCII: {})", i, available_key[i], static_cast<int>(available_key[i]));
                }
            }
            throw std::runtime_error("Missing key in row: " + key);
        }
        return strip_quotes(it->second); // Strip quotes from the value
    };

    try {
        auto change = get_value("Change %");
        stock.Date = get_value("Date");
        stock.Price = std::stod(get_value("Price"));
        stock.Open = std::stod(get_value("Open"));
        stock.High = std::stod(get_value("High"));
        stock.Low = std::stod(get_value("Low"));
        stock.Volume = get_value("Vol.");
        stock.ChangePercent = std::stod(change.substr(0, change.size() - 1)); // Remove "%" symbol and convert
    } catch (const std::exception& e) {
        throw std::runtime_error("Error in mapping row to StockPrice: " + std::string(e.what()));
    }

    return stock;
}

#endif // STOCK_PRICE_HPP
