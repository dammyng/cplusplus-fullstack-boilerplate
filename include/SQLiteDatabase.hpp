/**
#ifndef SQLITE_DATABASE_HPP
#define SQLITE_DATABASE_HPP

#include "IDatabase.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>

class SQLiteDatabase : public IDatabase {
public:
    explicit SQLiteDatabase(const std::string& dbPath);

    std::string getMessageById(int id) override;

private:
    std::shared_ptr<SQLite::Database> db_;
};

#endif // SQLITE_DATABASE_HPP
**/