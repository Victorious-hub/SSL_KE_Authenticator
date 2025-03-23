#include "./tcp_server/tcp_ssl_server.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <memory>

void handleClient(CTCPSSLServer& server, ASecureSocket::SSLSocket clientSocket) {
    std::cout << "New client connected!" << std::endl;

    while (true) {
        char buffer[256];
        int bytesReceived = server.Receive(clientSocket, buffer, sizeof(buffer), false);
        if (bytesReceived > 0) {
            std::string receivedMessage(buffer, bytesReceived);
            std::cout << "Received: " << receivedMessage << std::endl;

            std::string response = "Server received: " + receivedMessage;
            if (!server.Send(clientSocket, response)) {
                std::cerr << "Failed to send response to client." << std::endl;
                break;
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

    std::string port = "8080";
    CTCPSSLServer server(logger, port);

    server.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/server.crt");
    server.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/server.key");
    server.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    std::vector<std::thread> clientThreads;

    while (true) {
        ASecureSocket::SSLSocket clientSocket;
        if (server.Listen(clientSocket)) {
            clientThreads.emplace_back(std::thread(handleClient, std::ref(server), std::move(clientSocket)));
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