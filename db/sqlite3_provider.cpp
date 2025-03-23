#include <iostream>
#include <sstream>

#include <sqlite3.h>

#include "sqlite3_provider.h"
#include "logger.h"

SQLite3Provider::SQLite3Provider(const char *dbFile, const std::string& logFilename) : dbFile(dbFile), logger(logFilename) {
    int exit = sqlite3_open(dbFile, &db);
    if (exit) {
        logger.log(ERROR, "Error opening database: " + std::string(sqlite3_errmsg(db)));
        return;
    }
    logger.log(INFO, "Opened database successfully!");
}

SQLite3Provider::~SQLite3Provider() {
    sqlite3_close(db);
}


void SQLite3Provider::insert(const std::string& tableName, const std::string& values) {
    std::string sql = "INSERT INTO " + tableName + " VALUES (" + values + ");";
    char* errMsg = nullptr;
    int exit = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error inserting into table '" + tableName + "': " + errMsg);
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Inserted values into table '" + tableName + "' successfully.");
    }
}

void SQLite3Provider::update(const std::string& tableName, const std::string& values, const std::string& condition) {
    std::string sql = "UPDATE " + tableName + " SET " + values + " WHERE " + condition + ";";
    char* errMsg = nullptr;
    int exit = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error updating table '" + tableName + "': " + errMsg);
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Updated table '" + tableName + "' successfully.");
    }
}

void SQLite3Provider::remove(const std::string& tableName, const std::string& condition) {
    std::string sql = "DELETE FROM " + tableName + " WHERE " + condition + ";";
    char* errMsg = nullptr;
    int exit = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error deleting from table '" + tableName + "': " + errMsg);
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Deleted from table '" + tableName + "' successfully.");
    }
}

void SQLite3Provider::queryTable(const std::string& tableName) {
    std::string sql = "SELECT * FROM " + tableName + ";";
    char* errMsg = nullptr;

    auto callback = [](void* data, int argc, char** argv, char** colNames) -> int {
        Logger* logger = static_cast<Logger*>(data);
        std::ostringstream row;
        for (int i = 0; i < argc; i++) {
            row << colNames[i] << ": " << (argv[i] ? argv[i] : "NULL") << " | ";
        }
        logger->log(INFO, row.str());
        return 0;
    };

    int exit = sqlite3_exec(db, sql.c_str(), callback, &logger, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error querying table '" + tableName + "': " + errMsg);
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Queried table '" + tableName + "' successfully.");
    }
}

int main() {
    SQLite3Provider db("database", "logfile.txt");
    // db.insert("users", "'1', 'admin', 'password123'");
    db.queryTable("users");
    return 0;
}