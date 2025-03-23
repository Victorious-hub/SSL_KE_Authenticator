#ifndef SQLITE3_PROVIDER_H
#define SQLITE3_PROVIDER_H

#include <string>
#include <sqlite3.h>
#include "../utils/logger.h"

class SQLite3Provider {
public:
    SQLite3Provider(const char *dbFile, const std::string& logFilename);
    ~SQLite3Provider();

    void insert(const std::string& tableName, const std::string& values);
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