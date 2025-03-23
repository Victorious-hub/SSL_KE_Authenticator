#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>

#include "./tcp_server/tcp_ssl_server.h"
#include "./db/sqlite3_provider.h"
#include "./utils/token_manager.h"

void handleClient(CTCPSSLServer& server, ASecureSocket::SSLSocket clientSocket, SQLite3Provider& db, TokenManager& tokenManager) {
    std::cout << "New client connected!" << std::endl;

    while (true) {
        char buffer[256];
        int bytesReceived = server.Receive(clientSocket, buffer, sizeof(buffer), false);
        if (bytesReceived > 0) {
            std::string receivedMessage(buffer, bytesReceived);
            std::cout << "Received: " << receivedMessage << std::endl;

            if (receivedMessage.rfind("AUTH ", 0) == 0) 
            {
                std::string credentials = receivedMessage.substr(5);
                size_t spacePos = credentials.find(' ');
                if (spacePos == std::string::npos) {
                    std::cerr << "Invalid authentication request format." << std::endl;
                    server.Send(clientSocket, "ERROR: Invalid request format.");
                    continue;
                }

                std::string username = credentials.substr(0, spacePos);
                std::string password = credentials.substr(spacePos + 1);

                int userCount = db.isUserExists(username, password);

                if (userCount > 0) {
                    std::string token = tokenManager.generateToken(username);
                    std::cout << "Authentication successful. Generated token: " << token << std::endl;

                    server.Send(clientSocket, token);
                } else {
                    std::cerr << "Authentication failed for user: " << username << std::endl;
                    server.Send(clientSocket, "ERROR: Authentication failed.");
                }
            } else {
                std::cerr << "Invalid request received: " << receivedMessage << std::endl;
                server.Send(clientSocket, "ERROR: Invalid request.");
            }
        } else {
            std::cerr << "Client disconnected or error occurred." << std::endl;
            break;
        }
    }

    server.Disconnect(clientSocket);
    std::cout << "Client disconnected." << std::endl;
}

int main() {
    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    std::string port = "8081";
    CTCPSSLServer server(logger, port);
    SQLite3Provider db("/home/shyskov/Documents/libs/sem6/kursach/db/database", "/home/shyskov/Documents/libs/sem6/kursach/db/logfile.txt");
    TokenManager tokenManager;
    
    server.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/auth.crt");
    server.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/auth.key");
    server.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    std::vector<std::thread> clientThreads;

    while (true) {
        ASecureSocket::SSLSocket clientSocket;
        if (server.Listen(clientSocket)) {
            clientThreads.emplace_back(std::thread(handleClient, std::ref(server), std::move(clientSocket), std::ref(db), std::ref(tokenManager)));
        } else {
            std::cerr << "Failed to accept client connection." << std::endl;
        }
    }

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}