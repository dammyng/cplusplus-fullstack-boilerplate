#ifndef CSV_LOADER_HPP
#define CSV_LOADER_HPP

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>

// Generic CSV loader function template
template <typename T>
std::vector<T> load_csv(const std::string& filepath, std::function<T(const std::map<std::string, std::string>&)> row_mapper) {
    std::vector<T> data;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

        // Check and skip BOM (Byte Order Mark) if present
    char bom[3] = {0};
    file.read(bom, 3);
    if (!(bom[0] == char(0xEF) && bom[1] == char(0xBB) && bom[2] == char(0xBF))) {
        // If no BOM, reset file read pointer to the beginning
        file.seekg(0);
    }


    std::string line;
    std::vector<std::string> headers;

    // Read the headers
    spdlog::info("Parsed Headers:");
    if (std::getline(file, line)) {
        std::istringstream headerStream(line);
        std::string header;
        while (std::getline(headerStream, header, ',')) {
            headers.push_back(header);
            spdlog::info("  Header: '{}'", header); // Log each header
        }
    }

    // Read the rest of the lines as key-value pairs
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::map<std::string, std::string> row;
        std::string cell;
        size_t columnIndex = 0;

        while (std::getline(lineStream, cell, ',')) {
            if (columnIndex < headers.size()) {
                row[headers[columnIndex]] = cell;
            }
            ++columnIndex;
        }

        // Map the parsed row to a specific object of type T using the provided row_mapper function
        data.push_back(row_mapper(row));
    }

    return data;
}

#endif // CSV_LOADER_HPP