#ifndef RESPONSE_HELPER_HPP
#define RESPONSE_HELPER_HPP

#include "StandardResponse.hpp"

// Create a successful response with data
inline StandardResponse create_success_response(int code, const json& data, const json& meta = json::object()) {
    StandardResponse res;
    res.Ok = true;
    res.Code = code;
    res.Data = data;
    if (!meta.empty()) {
        res.Meta = meta;
    }
    return res;
}

// Create an error response with a message
inline StandardResponse create_error_response(int code, const std::string& error_message, const std::string& message = "") {
    StandardResponse res;
    res.Ok = false;
    res.Code = code;
    res.Error = error_message;
    if (!message.empty()) {
        res.Message = message;
    }
    return res;
}

// Create a response for not found
inline StandardResponse create_not_found_response(const std::string& target) {
    StandardResponse res;
    res.Ok = false;
    res.Code = 404;
    res.Message = "Resource not found";
    res.Error = "The requested resource '" + target + "' was not found.";
    return res;
}

// Create a response for internal server error
inline StandardResponse create_internal_server_error_response(const std::string& error_detail) {
    StandardResponse res;
    res.Ok = false;
    res.Code = 500;
    res.Message = "Internal Server Error";
    res.Error = "An unexpected error occurred.";
    res.Errors = error_detail; // Could be more detailed
    return res;
}

// **Alias create_server_error_response to create_internal_server_error_response**
inline StandardResponse create_server_error_response(const std::string& error_detail) {
    return create_internal_server_error_response(error_detail);
}

#endif