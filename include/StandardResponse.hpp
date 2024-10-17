// StandardResponse.hpp
#ifndef STANDARD_RESPONSE_HPP
#define STANDARD_RESPONSE_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <optional>

using json = nlohmann::json;

struct StandardResponse {
    bool Ok;
    int Code;
    std::optional<std::string> Message;
    std::optional<std::string> Error;
    std::optional<json> Errors;
    std::optional<json> Data;
    std::optional<json> Meta;

    // Serialize the struct to JSON
    json to_json() const {
        json j;
        j["ok"] = Ok;
        j["status_code"] = Code;

        if (Message.has_value()) {
            j["message"] = Message.value();
        }
        if (Error.has_value()) {
            j["error"] = Error.value();
        }
        if (Errors.has_value()) {
            j["errors"] = Errors.value();
        }
        if (Data.has_value()) {
            j["data"] = Data.value();
        }
        if (Meta.has_value()) {
            j["meta"] = Meta.value();
        }

        return j;
    }
};

#endif // STANDARD_RESPONSE_HPP