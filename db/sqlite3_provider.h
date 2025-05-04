#ifndef SQLITE3_PROVIDER_H
#define SQLITE3_PROVIDER_H

#include <ctime>
#include <string>
#include <sqlite3.h>
#include "../utils/logger.h"

class SQLite3Provider {
public:
    SQLite3Provider(const char *dbFile, const std::string& logFilename);
    ~SQLite3Provider();

    sqlite3* getDB(); // Expose the database connection
    bool getUsernameFromToken(const std::string& token, std::string& username);
    bool getTokenTTL(const std::string& token, std::time_t& ttl);
    void insert(const std::string& sql);
    void update(const std::string& tableName, const std::string& values, const std::string& condition);
    void remove(const std::string& tableName, const std::string& condition);
    void queryTable(const std::string& tableName);

    bool isUserExists(const std::string& username, const std::string& password); // better to do query_builder

private:
    sqlite3* db;
    std::string dbFile;
    Logger logger;
};

#endif // SQLITE3_PROVIDER_H