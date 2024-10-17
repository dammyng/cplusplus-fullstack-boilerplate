#ifndef IDATABASE_HPP
#define IDATABASE_HPP

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json; 

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual std::string getMessageById(int id) = 0;

    virtual json getData() = 0;

};

#endif