// Config.hpp
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <spdlog/spdlog.h>

class Config {
public:
    // Database Configuration
    std::string database_host;
    int database_port;
    std::string database_user;
    std::string database_password;
    std::string database_name;

    // Server Configuration
    std::string server_host;
    unsigned short server_port;
    std::string doc_root;
    unsigned short threads;

    // Application Configuration
    std::string log_level;
    std::string api_key;
    bool feature_flag;

    // JWT Configuration
    std::string jwt_secret;  
    std::string jwt_issuer;
    int jwt_expiration;

    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    void set_database_host(const std::string& host) { database_host = host; }
    void set_database_port(int port) { database_port = port; }
    void set_database_user(const std::string& user) { database_user = user; }
    void set_database_password(const std::string& password) { database_password = password; }
    void set_database_name(const std::string& name) { database_name = name; }

    void set_server_host(const std::string& host) { server_host = host; }
    void set_server_port(unsigned short port) { server_port = port; }
    void set_doc_root(const std::string& root) { doc_root = root; }
    void set_threads(unsigned short num_threads) { threads = num_threads; }

    void set_log_level(const std::string& level) { log_level = level; }
    void set_api_key(const std::string& key) { api_key = key; }
    void set_feature_flag(bool flag) { feature_flag = flag; }

    void set_jwt_secret(const std::string& secret) { jwt_secret = secret; }
    void set_jwt_issuer(const std::string& issuer) { jwt_issuer = issuer; }
    void set_jwt_expiration(int expiration) { jwt_expiration = expiration; }

private:
    Config() {
        loadConfig();
    }

    void loadConfig() {
        // Helper lambda to fetch environment variables
        auto get_env = [](const char* var, bool required = false, const std::string& default_val = "") -> std::string {
            const char* val = std::getenv(var);
            if (val) {
                return std::string(val);
            } else if (required) {
                spdlog::critical("Required environment variable {} is missing.", var);
                throw std::runtime_error("Missing required environment variables.");
            }
            return default_val;
        };

        // Database Configuration
        database_host = get_env("DATABASE_HOST", true);
        std::string port_str = get_env("DATABASE_PORT", true);
        try {
            database_port = std::stoi(port_str);
        } catch (const std::invalid_argument& e) {
            spdlog::critical("Invalid DATABASE_PORT value: {}", port_str);
            throw;
        }

        database_user = get_env("DATABASE_USER", true);
        database_password = get_env("DATABASE_PASSWORD", true);
        database_name = get_env("DATABASE_NAME", true);

        // Server Configuration
        server_host = get_env("SERVER_HOST", false, "0.0.0.0");
        std::string server_port_str = get_env("SERVER_PORT", false, "8080");
        try {
            server_port = static_cast<unsigned short>(std::stoi(server_port_str));
        } catch (const std::invalid_argument& e) {
            spdlog::warn("Invalid SERVER_PORT value: {}. Defaulting to 8080.", server_port_str);
            server_port = 8080;
        }

        doc_root = get_env("DOC_ROOT", false, "/var/www/");
        std::string threads_str = get_env("THREADS", false, "1");
        try {
            threads = static_cast<unsigned short>(std::stoi(threads_str));
        } catch (const std::invalid_argument& e) {
            spdlog::warn("Invalid THREADS value: {}. Defaulting to 1.", threads_str);
            threads = 1;
        }

        // Application Configuration
        log_level = get_env("LOG_LEVEL", false, "info");
        api_key = get_env("API_KEY", false, "");
        std::string feature_flag_str = get_env("FEATURE_FLAG", false, "false");
        feature_flag = (feature_flag_str == "true" || feature_flag_str == "1");

        // JWT Configuration
        jwt_secret = get_env("JWT_SECRET", true); 
        jwt_issuer = get_env("JWT_ISSUER", false, "your_app_name");
        std::string jwt_expiration_str = get_env("JWT_EXPIRATION", false, "3600");
        try {
            jwt_expiration = std::stoi(jwt_expiration_str);
        } catch (const std::invalid_argument& e) {
            spdlog::warn("Invalid JWT_EXPIRATION value: {}. Defaulting to 3600 seconds.", jwt_expiration_str);
            jwt_expiration = 3600;
        }

        // Configure spdlog based on LOG_LEVEL
        if (log_level == "debug") {
            spdlog::set_level(spdlog::level::debug);
        } else if (log_level == "info") {
            spdlog::set_level(spdlog::level::info);
        } else if (log_level == "warn") {
            spdlog::set_level(spdlog::level::warn);
        } else if (log_level == "error") {
            spdlog::set_level(spdlog::level::err);
        } else if (log_level == "critical") {
            spdlog::set_level(spdlog::level::critical);
        } else {
            spdlog::warn("Unknown LOG_LEVEL '{}', defaulting to 'info'.", log_level);
            spdlog::set_level(spdlog::level::info);
        }

        spdlog::info("Configuration loaded successfully.");
    }
};

#endif