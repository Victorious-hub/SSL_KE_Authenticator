#ifndef SQLITE3_PROVIDER_H
#define SQLITE3_PROVIDER_H

#include <string>
#include <sqlite3.h>
#include "logger.h"

class SQLite3Provider {
public:
    SQLite3Provider(const char *dbFile, const std::string& logFilename); // Declaration only
    ~SQLite3Provider(); // Declaration only

    void insert(const std::string& tableName, const std::string& values);
    void update(const std::string& tableName, const std::string& values, const std::string& condition);
    void remove(const std::string& tableName, const std::string& condition);
    void queryTable(const std::string& tableName);

private:
    sqlite3* db;
    std::string dbFile;
    Logger logger;
};

#endif // SQLITE3_PROVIDER_H