#ifndef POSTGRES_DATABASE_HPP
#define POSTGRES_DATABASE_HPP

#include "IDatabase.hpp"
#include <pqxx/pqxx> 
#include <string>
#include <memory>

class PostgresDatabase : public IDatabase {
public:
    explicit PostgresDatabase(const std::string& connectionString);

    std::string getMessageById(int id) override;
    json getData() override;

private:
    std::string connectionString_;
    std::unique_ptr<pqxx::connection> conn_;  

};

#endif