/**
 * #include "SQLiteDatabase.hpp"
#include <iostream>

SQLiteDatabase::SQLiteDatabase(const std::string& dbPath) {
    try {
        db_ = std::make_shared<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE);
    } catch (const std::exception& e) {
        std::cerr << "Failed to open SQLite database: " << e.what() << std::endl;
        throw;
    }
}

std::string SQLiteDatabase::getMessageById(int id) {
    try {
        SQLite::Statement query(*db_, "SELECT content FROM messages WHERE id = ?");
        query.bind(1, id);

        if (query.executeStep()) {
            return query.getColumn(0).getString();
        } else {
            return "Message not found.";
        }
    } catch (const std::exception& e) {
        std::cerr << "SQLite query error: " << e.what() << std::endl;
        return "Database error.";
    }
}

**/