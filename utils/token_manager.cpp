#include <iostream>
#include "token_manager.h"

TokenManager::TokenManager() {
    this->secret = "example";
}

TokenManager::~TokenManager() {}

std::string TokenManager::generateToken(std::string& header) {
    return header + "." + "test" + "." + "test" + "." + this->secret;
}

std::string TokenManager::parseToken(const std::string& token) {
    size_t firstPeriod = token.find('.');
    size_t secondPeriod = token.find('.', firstPeriod + 1);
    return token.substr(firstPeriod + 1, secondPeriod - firstPeriod - 1);
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

bool TokenManager::VerifyToken(const std::string& token, const std::string& signature) {
    size_t lastPeriod = token.rfind('.'); // Find the last period in the token
    if (lastPeriod == std::string::npos) {
        return false; // Invalid token format
    }

    std::string tokenPart = trim(token.substr(lastPeriod + 1));
    std::string trimmedSignature = trim(signature);

    std::cout << "Token part: [" << tokenPart << "], Secret: [" << this->secret << "]" << std::endl;
    std::cout << "Comparison result: " << (tokenPart == trimmedSignature) << std::endl;

    return tokenPart == trimmedSignature; // Compare the trimmed strings
}