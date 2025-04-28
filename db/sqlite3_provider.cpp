#include <iostream>
#include <sstream>
#include <ctime>
#include <sqlite3.h>

#include "sqlite3_provider.h"
#include "../utils/logger.h"

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

void SQLite3Provider::insert(const std::string& sql) {
    char* errMsg = nullptr;
    int exit = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error executing insert query: " + std::string(errMsg));
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Insert query executed successfully.");
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

bool SQLite3Provider::isUserExists(const std::string& username, const std::string& password) {
    std::string sql = "SELECT EXISTS(SELECT 1 FROM users WHERE username = '" + username + "' AND password = '" + password + "');";
    char* errMsg = nullptr;
    bool exists = false;

    auto callback = [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) { // Ensure argv[0] is not NULL
            bool* exists = static_cast<bool*>(data);
            *exists = std::stoi(argv[0]) != 0; // Convert the result to a boolean
        }
        return 0;
    };

    int exit = sqlite3_exec(db, sql.c_str(), callback, &exists, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error checking if user exists: " + std::string(errMsg));
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Checked if user exists successfully.");
    }

    std::cout << sql << " -> Result: " << (exists ? 1 : 0) << std::endl; // Debug output

    return exists;
}


bool SQLite3Provider::getTokenTTL(const std::string& token, std::time_t& ttl) {
    std::string sql = "SELECT ttl FROM tokens WHERE token = '" + token + "';";
    char* errMsg = nullptr;
    bool tokenExists = false;

    auto callback = [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) {
            std::time_t* ttl = static_cast<std::time_t*>(data);
            *ttl = std::stoll(argv[0]); // Convert the TTL value to a time_t
            return 0;
        }
        return 1; // No rows found
    };

    int exit = sqlite3_exec(db, sql.c_str(), callback, &ttl, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error querying token TTL: " + std::string(errMsg));
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Queried token TTL successfully.");
        tokenExists = true;
    }

    return tokenExists;
}


bool SQLite3Provider::getUsernameFromToken(const std::string& token, std::string& username) {
    std::string sql = "SELECT username FROM tokens WHERE token = '" + token + "';";
    char* errMsg = nullptr;
    bool tokenExists = false;

    auto callback = [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) {
            std::string* username = static_cast<std::string*>(data);
            *username = argv[0];
            return 0;
        }
        return 1; // No rows found
    };

    int exit = sqlite3_exec(db, sql.c_str(), callback, &username, &errMsg);
    if (exit != SQLITE_OK) {
        logger.log(ERROR, "Error querying username from token: " + std::string(errMsg));
        sqlite3_free(errMsg);
    } else {
        logger.log(INFO, "Queried username from token successfully.");
        tokenExists = true;
    }

    return tokenExists;
}