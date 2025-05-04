#include <iostream>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <sqlite3.h>

#include "./db/sqlite3_provider.h"

// Function to hash a password using SHA-256
std::string hashPassword1(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

// Function to create the `users` table
void createUsersTable(SQLite3Provider& dbProvider) {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            password TEXT NOT NULL
        );
    )";

    dbProvider.insert(sql);
    std::cout << "Users table created successfully." << std::endl;
}

// Function to add a new user with a hashed password
bool addUser(SQLite3Provider& dbProvider, const std::string& username, const std::string& password) {
    std::string hashedPassword = hashPassword1(password);
    std::string sql = "INSERT INTO users (username, password) VALUES ('" + username + "', '" + hashedPassword + "');";

    try {
        dbProvider.insert(sql);
        std::cout << "User added successfully: " << username << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error adding user: " << e.what() << std::endl;
        return false;
    }
}

// Function to verify a user's password
bool verifyUser(SQLite3Provider& dbProvider, const std::string& username, const std::string& password) {
    std::string hashedPassword = hashPassword1(password);
    std::string sql = "SELECT EXISTS(SELECT 1 FROM users WHERE username = '" + username + "' AND password = '" + hashedPassword + "');";

    char* errMsg = nullptr;
    bool exists = false;

    auto callback = [](void* data, int argc, char** argv, char** colNames) -> int {
        if (argc > 0 && argv[0]) {
            bool* exists = static_cast<bool*>(data);
            *exists = std::stoi(argv[0]) != 0;
        }
        return 0;
    };

    int exit = sqlite3_exec(dbProvider.getDB(), sql.c_str(), callback, &exists, &errMsg);
    if (exit != SQLITE_OK) {
        std::cerr << "Error verifying user: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "User verification query executed successfully." << std::endl;
    }

    return exists;
}

int main() {
    SQLite3Provider db("/home/shyskov/Documents/libs/sem6/kursach/db/database", "/home/shyskov/Documents/libs/sem6/kursach/db/logfile.txt");

    // Add users
    // addUser(db, "john_doe", "secure_password");
    // addUser(db, "jane_doe", "another_password");

    // Verify users
    if (verifyUser(db, "john_doe", "secure_password")) {
        std::cout << "User john_doe verified successfully!" << verifyUser(db, "john_doe", "secure_password") << std::endl;
    } else {
        std::cout << "Failed to verify user john_doe." << std::endl;
    }

    if (verifyUser(db, "jane_doe", "wrong_password")) {
        std::cout << "User jane_doe verified successfully!" << std::endl;
    } else {
        std::cout << "Failed to verify user jane_doe." << std::endl;
    }

    return 0;
}