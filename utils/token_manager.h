#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include <iostream>

class TokenManager {
public:
    TokenManager();
    ~TokenManager();

    std::string generateToken(std::string& header);
    std::string parseToken(const std::string& token);

    bool VerifyToken(const std::string& token, const std::string& signature);
private:
    std::string secret;
};

#endif // TOKEN_MANAGER_H