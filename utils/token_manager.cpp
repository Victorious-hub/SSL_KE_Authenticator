#include <iostream>
#include "token_manager.h"

TokenManager::TokenManager() {
    this->secret = "example";
}

TokenManager::~TokenManager() {}

std::string TokenManager::generateToken(std::string& header) {
    return header + "." + "test" + "." + "test";
}

std::string TokenManager::parseToken(const std::string& token) {
    size_t firstPeriod = token.find('.');
    size_t secondPeriod = token.find('.', firstPeriod + 1);
    return token.substr(firstPeriod + 1, secondPeriod - firstPeriod - 1);
}

bool TokenManager::VerifyToken(const std::string& token, const std::string& signature) {
    size_t secondPeriod = token.find('.', token.find('.') + 1);
    return token.substr(secondPeriod + 1) == signature;
}
