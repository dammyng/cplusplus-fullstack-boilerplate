#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "listener.hpp"
#include "PostgresDatabase.hpp"
#include "Config.hpp" 
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <nlohmann/json.hpp> 
#include "CustomFormatter.hpp" 

using json = nlohmann::json;
namespace net = boost::asio;
namespace po = boost::program_options;
using tcp = net::ip::tcp;

int main(int argc, char *argv[])
{
    try
    {
        Config& config = Config::getInstance();

        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        spdlog::info("Starting Web Server");

        // Command-line options
        po::options_description desc("Allowed command line options");
        desc.add_options()
            ("help", "produce help message")
            ("host", po::value<std::string>(), "host address to bind to")
            ("port", po::value<unsigned short>(), "port to expose to")
            ("doc_root", po::value<std::string>(), "root directory to serve")
            ("threads", po::value<unsigned short>(), "number of threads to use");

        po::variables_map args;
        po::store(po::parse_command_line(argc, argv, desc), args);
        po::notify(args);

        if (args.count("help"))
        {
            std::cout << desc << "\n";
            return EXIT_SUCCESS;
        }

 
        // Define server configurations with defaults from Config
        std::string host = args.count("host") ? args["host"].as<std::string>() : "0.0.0.0";
        unsigned short port = args.count("port") ? args["port"].as<unsigned short>() : 8080;
        std::string doc_root = args.count("doc_root") ? args["doc_root"].as<std::string>() : "/var/www/";
        unsigned short threads = args.count("threads") ? args["threads"].as<unsigned short>() : 1;

        spdlog::info("Server Configuration - Host: {}, Port: {}, Document Root: {}, Threads: {}", host, port, doc_root, threads);

        // Initialize Boost.Asio I/O context
        net::io_context ioc{threads};

        std::string connStr = "dbname=" + config.database_name +
                               " user=" + config.database_user +
                               " password=" + config.database_password +
                               " host=" + config.database_host +
                               " port=" + std::to_string(config.database_port);

        spdlog::info("PostgreSQL Connection String: {}", connStr);

        auto db = std::make_shared<PostgresDatabase>(connStr);

        // Create and launch a listening port
        std::make_shared<listener>(
            ioc,
            tcp::endpoint{net::ip::make_address(host), port},
            std::make_shared<std::string>(doc_root),
            db)
            ->run();

        spdlog::info("Listener started on {}:{}", host, port);

        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for (auto i = threads - 1; i > 0; --i)
        {
            v.emplace_back(
                [&ioc]
                {
                    ioc.run();
                });
        }
        spdlog::info("Main thread running IO context");

        ioc.run();
        spdlog::info("Application shutting down");

        // Join all threads
        for (auto& t : v) {
            if (t.joinable()) {
                t.join();
            }
        }

        spdlog::info("Application exited successfully");
    }
    catch (const std::exception& e)
    {
        spdlog::critical("Application error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}