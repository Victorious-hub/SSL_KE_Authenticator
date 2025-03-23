#include "./tcp_client/tcp_ssl_client.h"
#include <iostream>
#include <string>

std::string authenticateWithAuthServer(CTCPSSLClient& proxyClient, const std::string& username, const std::string& password) {
    std::string authRequest = "AUTH " + username + " " + password;
    if (!proxyClient.Send(authRequest)) {
        std::cerr << "Failed to send authentication request to auth_server." << std::endl;
        return "";
    }

    char buffer[1024] = {0};
    int bytesReceived = proxyClient.Receive(buffer, sizeof(buffer), false);
    if (bytesReceived > 0) {
        std::string token(buffer, bytesReceived);
        std::cout << "Received token from auth_server: " << token << std::endl;
        return token;
    } else {
        std::cerr << "Failed to receive token from auth_server." << std::endl;
        return "";
    }
}

int main() {
    std::string proxyAddress = "127.0.0.1";
    std::string proxyPort = "9090";

    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    CTCPSSLClient proxyClient(logger);
    proxyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/client.crt");
    proxyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/client.key");
    proxyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    if (proxyClient.Connect(proxyAddress, proxyPort)) {
        std::cout << "Connected to proxy at " << proxyAddress << ":" << proxyPort << std::endl;

        std::string username, password;
        std::cout << "Enter username: ";
        std::getline(std::cin, username);
        std::cout << "Enter password: ";
        std::getline(std::cin, password);

        std::string token = authenticateWithAuthServer(proxyClient, username, password);
        if (token.empty()) {
            std::cerr << "Authentication failed. Exiting..." << std::endl;
            return 1;
        }

        while (true) {
            std::cout << "Enter a message to send to the server (or type 'exit' to quit): ";
            std::string userInput;
            std::getline(std::cin, userInput);

            if (userInput == "exit") {
                std::cout << "Exiting client..." << std::endl;
                break;
            }

            std::string message = "Token: " + token + "\nMessage: " + userInput;
            if (proxyClient.Send(message)) {
                std::cout << "Message sent to server: " << message << std::endl;
            } else {
                std::cerr << "Failed to send message to server." << std::endl;
                break;
            }
            char buffer[1024] = {0};
            int bytesReceived = proxyClient.Receive(buffer, sizeof(buffer), false);
            if (bytesReceived > 0) {
                std::cout << "Server says: " << std::string(buffer, bytesReceived) << std::endl;
            } else {
                std::cerr << "Failed to receive message from server." << std::endl;
                break;
            }
        }

        proxyClient.Disconnect();
    } else {
        std::cerr << "Failed to connect to proxy." << std::endl;
    }

    return 0;
}